#include <algorithm>
#include <cmath>

#include "esp_log.h"

#include "Status.hpp"
#include "def.h"
#include "i2s_rx_slot.h"
#include "mic_reader.h"
#include "mic_proc.h"
#include "kws_preprocessor.h"

static const char *TAG = "mic_reader";

#define FILTER_INIT_FRAME_NUM 30
#define MIC_TEST_FRAME_NUM 10

// high pass (120Hz) filter coefficients (Q30)
static const int32_t b_coeffs[] = {1049020290, -1049020290, 0};
static const int32_t a_coeffs[] = {1073741824, 1024298756, 0};
static biquad_df1_o1_filter s_filter(b_coeffs, a_coeffs, 30);

static size_t s_silence_env = 0;

static size_t max_abs_arr[DET_VOICED_FRAMES_WINDOW] = {0};
static uint8_t is_speech_arr[DET_VOICED_FRAMES_WINDOW] = {0};
static uint8_t current_frames[DET_VOICED_FRAMES_WINDOW * FRAME_SZ] = { 0 };

#define PROC_FRAME_SZ FRAME_LEN * HW_ELEM_BYTES

static int32_t convert_hw_bytes(uint8_t *buf, size_t bytes_width) {
  int32_t val = (buf[bytes_width - 1] & 0x80) ? -1 : 0;
  for(int b = bytes_width - 1; b >= 0; b-- )
    val = (val << 8) | (buf[b] & 0xFF);
  return val;
}

static int read_frame(audio_t *dst, biquad_df1_o1_filter &filter, size_t &max_abs) {
  size_t item_size = 0;
  uint8_t *item = (uint8_t *)xRingbufferReceiveUpTo(xMicRingBuffer, &item_size, portMAX_DELAY, PROC_FRAME_SZ);
  if (item != NULL && item_size == PROC_FRAME_SZ) {
    ESP_LOGV(TAG, "recv bytes=%d", item_size);

    for (size_t j = 0; j < FRAME_LEN; j++) {
      int32_t val = convert_hw_bytes(&item[j * HW_ELEM_BYTES], HW_ELEM_BYTES);
      dst[j] = filter.proc_val(val);
      const size_t abs_val = abs(dst[j]); 
      if (abs_val > max_abs) {
        max_abs = abs_val;
      }
    }
    vRingbufferReturnItem(xMicRingBuffer, (void *)item);
    return 0;
  } else {
    ESP_LOGE(TAG, "failed to receive item");
    return -1;
  }
};

static void mic_proc_task(void *pv) {
  size_t env = 0;
  size_t silence_env_acc = 0;
  size_t silence_env_frames_counter = 0;
  size_t cur_frame = 0;
  size_t num_voiced = 0;
  uint8_t trig = 0;
  KWSWordDesc_t word;

  for (size_t i = 0; i < DET_VOICED_FRAMES_WINDOW; i++) {
    audio_t *proc_data = (audio_t*)&current_frames[(i % DET_VOICED_FRAMES_WINDOW) * FRAME_SZ];
    size_t max_abs = 0;
    if (read_frame(proc_data, s_filter, max_abs) < 0) {
      continue;
    }
    env += max_abs;
    max_abs_arr[i] = max_abs;
  }
  env = env / DET_VOICED_FRAMES_WINDOW;
  s_silence_env = env;
  ESP_LOGI(TAG, "init env=%d", env);

  for (;;) {
    audio_t *proc_data = (audio_t*)&current_frames[(cur_frame % DET_VOICED_FRAMES_WINDOW) * FRAME_SZ];
    size_t max_abs = 0;
    if (read_frame(proc_data, s_filter, max_abs) < 0) {
      continue;
    }

    max_abs_arr[cur_frame % DET_VOICED_FRAMES_WINDOW] = max_abs;

    const uint8_t is_speech = max_abs > env * DET_IS_VOICED_THRESHOLD;
    num_voiced += is_speech;
    is_speech_arr[cur_frame % DET_VOICED_FRAMES_WINDOW] = is_speech;

    if (!trig) {
      if (num_voiced >= DET_VOICED_FRAMES_THRESHOLD) {
        word.frame_num = 0;
        word.max_abs = 0;
        trig = 1;
        silence_env_frames_counter = 0;
        silence_env_acc = 0;
        ESP_LOGD(TAG, "__start[%d]=%d, max_abs=%d, env=%d, s_silence_env=%d", cur_frame - DET_VOICED_FRAMES_WINDOW, cur_frame, max_abs, env, s_silence_env);
        for (size_t k = 1; k <= DET_VOICED_FRAMES_WINDOW; k++) {
          const size_t frame_num = (cur_frame + k) % DET_VOICED_FRAMES_WINDOW;
          audio_t* frame_ptr = (audio_t*)&current_frames[frame_num * FRAME_SZ];
          const auto xBytesSent = xStreamBufferSend(xKWSFramesBuffer, frame_ptr, FRAME_SZ, 0);
          if (xBytesSent < FRAME_SZ) {
            ESP_LOGW(TAG, "xKWSFramesBuffer: xBytesSent=%d (%d)", xBytesSent, FRAME_SZ);
          }
          word.frame_num++;
          word.max_abs = std::max(word.max_abs, max_abs_arr[frame_num]);
        }
      } else {
        env = std::max(max_abs_arr[(cur_frame + 1)  % DET_VOICED_FRAMES_WINDOW], s_silence_env);
        if (silence_env_frames_counter++ >= DET_SILENCE_ENV_FRAME_NUM) {
          s_silence_env = silence_env_acc / DET_SILENCE_ENV_FRAME_NUM;
          ESP_LOGD(TAG, "s_silence_env=%d", s_silence_env);
          xEventGroupClearBits(xStatusEventGroup, STATUS_MIC_ENV_BITS_MSK);
          if (s_silence_env > 300) {
            xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_BAD_ENV_MSK);
          } else if (s_silence_env > 100) {
            xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_NOISY_ENV_MSK);
          }
          silence_env_frames_counter = 0;
          silence_env_acc = 0;
        } else {
          silence_env_acc += max_abs;
        }
      }
    } else {
      if (num_voiced <= DET_UNVOICED_FRAMES_THRESHOLD) {
        trig = 0;
        ESP_LOGD(TAG, "__end[%d]=%d, max_abs=%d, env=%d, s_silence_env=%d", cur_frame - DET_VOICED_FRAMES_WINDOW, cur_frame, max_abs, env, s_silence_env);
        xQueueSend(xKWSWordQueue, &word, 0);
      } else {
        word.frame_num++;
        word.max_abs = std::max(word.max_abs, max_abs);
        const auto xBytesSent = xStreamBufferSend(xKWSFramesBuffer, proc_data, FRAME_SZ, 0);
        if (xBytesSent < FRAME_SZ) {
          ESP_LOGW(TAG, "xKWSFramesBuffer: xBytesSent=%d (%d)", xBytesSent, FRAME_SZ);
        }
      }
    }

    num_voiced -= is_speech_arr[(cur_frame + 1) % DET_VOICED_FRAMES_WINDOW];
    
    cur_frame++;
  }
}

static float compute_mean(const audio_t *data, size_t samples) {
  int32_t sum = 0;
  for (size_t i = 0; i < samples; i++) {
    sum += data[i];
  }
  return sum / int32_t(samples);
}

static float compute_std_dev(const audio_t *data, size_t samples, float mean_val) {
  float sum_sq = 0;
  for (size_t i = 0; i < samples; i++) {
    sum_sq += std::pow(data[i] - mean_val, 2);
  }
  return std::sqrt(sum_sq / float(samples));
}

static bool test_microphone(const audio_t *data, size_t samples) {
  const auto mean = compute_mean(data, samples);
  const auto std_dev = compute_std_dev(data, samples, mean);
  ESP_LOGV(TAG, "mean=%f, std_dev=%f", mean, std_dev);
  if (abs(mean) > 1000) {
    return false;
  }
  const size_t mic_data_def_std_dev = 1 << (8 * HW_ELEM_BYTES - 3); // 1/4 of dynamic range
  if (std_dev > mic_data_def_std_dev) {
    return false;
  }
  return true;
}

bool mic_reader_init() {
  i2s_receiver_init();
  mic_conf_t mic_conf[2] = {
    {
      .sample_rate = KWS_SAMPLE_RATE,
      .slot_type = stStdRight,
    },
    {
      .sample_rate = KWS_SAMPLE_RATE,
      .slot_type = stStdLeft,
    },
  };

  bool init_result = false;
  for (size_t i = 0; i < sizeof(mic_conf) / sizeof(mic_conf[0]); ++i) {
    ESP_LOGD(TAG, "Try conf[%d]", i);
    i2s_rx_slot_init(mic_conf[i]);
    i2s_rx_slot_start();

    if (i == 0) {
      // read first bad samples, init filter
      for (size_t i = 0; i < FILTER_INIT_FRAME_NUM; i++) {
        audio_t *proc_data = (audio_t*)&current_frames[0];
        size_t max_abs = 0;
        read_frame(proc_data, s_filter, max_abs);
      }
    }

    size_t bad_frames = 0;
    for (size_t i = 0; i < MIC_TEST_FRAME_NUM; i++) {
      audio_t *proc_data = (audio_t*)&current_frames[0];
      size_t max_abs = 0;
      read_frame(proc_data, s_filter, max_abs);
      bad_frames += test_microphone(proc_data, FRAME_LEN) == false;
    }
    if (bad_frames == 0) {
      i2s_rx_slot_stop();
      init_result = true;
      break;
    } else {
      i2s_rx_slot_stop();
      i2s_rx_slot_release();
    }
  }
  if (!init_result) {
    ESP_LOGE(TAG, "Unable to init microphone");
    return false;
  }

  auto xReturned =
    xTaskCreate(mic_proc_task, "mic_proc_task", configMINIMAL_STACK_SIZE + 1024 * 16, NULL, 2, NULL);
  if (xReturned != pdPASS) {
    ESP_LOGE(TAG, "Error creating mic_proc_task");
    return false;
  }
  return true;
}

void mic_reader_release() {
  i2s_rx_slot_release();
}

size_t mic_reader_get_env() {
  return s_silence_env;
}