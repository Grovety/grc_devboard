#include "esp_log.h"
#include "esp_timer.h"

#include "def.h"
#include "mfcc.h"
#include "kws_preprocessor.h"
#include "i2s_rx_slot.h"
#include "mic_reader.h"
#include "Status.hpp"

static const char *TAG = "kws_preprocessor";

SemaphoreHandle_t xKWSSema;
SemaphoreHandle_t xKWSWordSema;
QueueHandle_t xKWSWordQueue;
QueueHandle_t xKWSRequestQueue;
QueueHandle_t xKWSResultQueue;
StreamBufferHandle_t xKWSFramesBuffer;

#define MAX_KWS_WORDS         4
#define KWS_FRAME_SZ          KWS_FRAME_LEN * ELEM_BYTES
#define KWS_FRAME_SHIFT_BYTES KWS_FRAME_SHIFT * ELEM_BYTES
#define MFCC_BUF_SZ           (KWS_FRAME_NUM * KWS_NUM_MFCC * sizeof(float))
#define MFCC_PROC_FRAME_NUM   (KWS_FRAME_SHIFT / FRAME_LEN)

#define PROC_BUF_SZ           KWS_FRAME_SZ
#define PROC_BUF_FRAME_NUM    (PROC_BUF_SZ / FRAME_SZ)
#define STREAM_BUF_FRAME_NUM  (1 * KWS_SAMPLE_RATE / FRAME_LEN)
#define STREAM_BUF_SZ         (STREAM_BUF_FRAME_NUM * FRAME_SZ)

static uint8_t proc_buf[PROC_BUF_SZ] = {0};

void kws_task(void *pv) {
  static MFCC s_mfcc_pp(KWS_NUM_MFCC, KWS_FRAME_LEN);

  for (;;) {
    size_t req_words = 0;
    xQueuePeek(xKWSRequestQueue, &req_words, portMAX_DELAY);

    if (req_words > MAX_KWS_WORDS) {
      ESP_LOGE(TAG, "req_words > MAX_KWS_WORDS");
    }
    ESP_LOGD(TAG, "recogninze req_words=%d", req_words);

    float *mfcc_buffer = new float[req_words * KWS_FRAME_NUM * KWS_NUM_MFCC];
    memset(mfcc_buffer, 0, req_words * MFCC_BUF_SZ);

    i2s_rx_slot_start();
    ESP_LOGI(TAG, "env=%d", mic_reader_get_env());
    size_t det_words = 0;
    for (; det_words < req_words;) {
      KWSWordDesc_t word = {.frame_num=0, .max_abs=0};
      while (xQueueReceive(xKWSWordQueue, &word, pdMS_TO_TICKS(1)) == pdFAIL) {
        if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
          // canceled request
          xQueueReset(xKWSResultQueue);
          break;
        }
      }
      if (word.frame_num == 0) {
        goto CLEANUP;
      } else if (word.frame_num > 0) {
        ESP_LOGD(TAG, "got word: frame_num=%d, max_abs=%d", word.frame_num, word.max_abs);
      }
      xSemaphoreGive(xKWSWordSema);
      float *mfcc_coeffs = &mfcc_buffer[det_words * KWS_FRAME_NUM * KWS_NUM_MFCC];
      memset(proc_buf, 0, PROC_BUF_SZ);

      const int64_t t1 = esp_timer_get_time();
      const size_t mfcc_frames = std::min((word.frame_num + MFCC_PROC_FRAME_NUM - 1) / MFCC_PROC_FRAME_NUM, size_t(KWS_FRAME_NUM));
      ESP_LOGD(TAG, "mfcc_frames=%d", mfcc_frames);

      auto xReceivedBytes = xStreamBufferReceive(xKWSFramesBuffer, proc_buf, PROC_BUF_SZ, 0);
      ESP_LOGD(TAG, "recv bytes=%d", xReceivedBytes);

      s_mfcc_pp.MfccCompute((audio_t*)proc_buf, mfcc_coeffs, word.max_abs);
      memmove(proc_buf, &proc_buf[KWS_FRAME_SHIFT_BYTES], KWS_FRAME_SHIFT_BYTES);
      memset(&proc_buf[KWS_FRAME_SHIFT_BYTES], 0, KWS_FRAME_SHIFT_BYTES);

      size_t proc_frames = 1;
      for (; proc_frames < mfcc_frames; proc_frames++) {
        audio_t *half_proc_buf = (audio_t*)&proc_buf[KWS_FRAME_SHIFT_BYTES];
        const auto xReceivedBytes = xStreamBufferReceive(xKWSFramesBuffer, half_proc_buf, KWS_FRAME_SHIFT_BYTES, 0);
        ESP_LOGD(TAG, "recv bytes=%d", xReceivedBytes);

        s_mfcc_pp.MfccCompute((audio_t*)proc_buf, &mfcc_coeffs[proc_frames * KWS_NUM_MFCC], word.max_abs);

        memmove(proc_buf, half_proc_buf, xReceivedBytes);
        memset(&proc_buf[xReceivedBytes], 0, KWS_FRAME_SHIFT_BYTES - xReceivedBytes);
      }
      for (size_t i = mfcc_frames; i < KWS_FRAME_NUM; i++) {
        memset(proc_buf, 0, PROC_BUF_SZ);
        audio_t *audio_data = (audio_t*)proc_buf;
        s_mfcc_pp.MfccCompute(audio_data, &mfcc_coeffs[i * KWS_NUM_MFCC], word.max_abs);
      }
      ESP_LOGD(TAG, "preproc %d frames[%d]=%lld us", KWS_FRAME_NUM, det_words, esp_timer_get_time() - t1);

      if (word.frame_num > STREAM_BUF_FRAME_NUM) {
        xStreamBufferReset(xKWSFramesBuffer);
        ESP_LOGW(TAG, "cleared %d frames", word.frame_num - STREAM_BUF_FRAME_NUM);
      }
      det_words++;
    }
    ESP_LOGD(TAG, "detected words %d out of %d requested", det_words, req_words);

    xEventGroupSetBits(xStatusEventGroup, STATUS_KWS_BUSY_MSK);
    for (size_t i = 0; i < det_words; i++) {
      float *mfcc_coeffs = &mfcc_buffer[i * KWS_FRAME_NUM * KWS_NUM_MFCC];
      char result[32] = {0};
      const int64_t t2 = esp_timer_get_time();
      const int category = kws_recognize_word(mfcc_coeffs, KWS_FEATURES_LEN);
      xQueueSend(xKWSResultQueue, &category, 0);
      kws_get_category(category, result, sizeof(result));
      ESP_LOGI(TAG, ">> result=%s, kws[%d]=%lld us", result, det_words, esp_timer_get_time() - t2);
    }
    xEventGroupClearBits(xStatusEventGroup, STATUS_KWS_BUSY_MSK);

CLEANUP:
    i2s_rx_slot_stop();
    xSemaphoreTake(xKWSWordSema, 0);
    xQueueReset(xKWSWordQueue);
    xStreamBufferReset(xKWSFramesBuffer);
    xQueueReceive(xKWSRequestQueue, &req_words, 0);
    delete[] mfcc_buffer;
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_KWS_STOP_MSK);
  }
}

int kws_preprocessor_init() {
  ESP_LOGD(TAG, "KWS_FRAME_LEN=%d, KWS_FRAME_SHIFT=%d, KWS_FRAME_NUM=%d", KWS_FRAME_LEN, KWS_FRAME_SHIFT, KWS_FRAME_NUM);
  ESP_LOGD(TAG, "KWS_FRAME_SZ=%d, KWS_FRAME_SHIFT_BYTES=%d", KWS_FRAME_SZ, KWS_FRAME_SHIFT_BYTES);
  ESP_LOGD(TAG, "PROC_BUF_FRAME_NUM=%d, STREAM_BUF_FRAME_NUM=%d", PROC_BUF_FRAME_NUM, STREAM_BUF_FRAME_NUM);

  xKWSSema = xSemaphoreCreateBinary();
  if (xKWSSema) {
    xSemaphoreGive(xKWSSema);
  } else {
    ESP_LOGE(TAG, "Error creating xKWSSema");
  }
  xKWSWordSema = xSemaphoreCreateBinary();
  if (!xKWSWordSema) {
    ESP_LOGE(TAG, "Error creating xKWSWordSema");
  }

  xKWSWordQueue = xQueueCreate(MAX_KWS_WORDS, sizeof(KWSWordDesc_t));
  if (xKWSWordQueue == NULL) {
    ESP_LOGE(TAG, "Error creating KWS word queue");
    return -1;
  }

  xKWSFramesBuffer = xStreamBufferCreate(STREAM_BUF_SZ, FRAME_SZ);
  if (xKWSFramesBuffer == NULL) {
    ESP_LOGE(TAG, "Error creating stream buffer");
    return -1;
  }
  xKWSResultQueue = xQueueCreate(MAX_KWS_WORDS, sizeof(int));
  if (xKWSResultQueue == NULL) {
    ESP_LOGE(TAG, "Error creating KWS result queue");
    return -1;
  }
  xKWSRequestQueue = xQueueCreate(1, sizeof(size_t));
  if (xKWSRequestQueue == NULL) {
    ESP_LOGE(TAG, "Error creating KWS word queue");
    return -1;
  }

  auto xReturned =
    xTaskCreate(kws_task, "kws_task", configMINIMAL_STACK_SIZE + 1024 * 16, NULL, 1, NULL);
  if (xReturned != pdPASS) {
    ESP_LOGE(TAG, "Error creating kws_task");
  }
  return 0;
}

void kws_req_word(size_t req_words) {
  if (xQueueSend(xKWSRequestQueue, &req_words, 0) == pdPASS) {
    xEventGroupClearBits(xStatusEventGroup, STATUS_EVENT_KWS_STOP_MSK);
  }
}

void kws_req_cancel() {
  xQueueReset(xKWSRequestQueue);
  xEventGroupWaitBits(xStatusEventGroup, STATUS_EVENT_KWS_STOP_MSK, pdTRUE, pdFALSE, portMAX_DELAY);
}
