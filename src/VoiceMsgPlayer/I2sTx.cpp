#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

#include "I2sTx.hpp"

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

static const char *TAG = "I2sTx";

// I2S Configuration
#define I2S_BLK_PIN GPIO_NUM_40
#define I2S_WS_PIN GPIO_NUM_39
#define I2S_DATA_OUT_PIN GPIO_NUM_41
#define I2S_DATA_IN_PIN I2S_GPIO_UNUSED
#define I2S_SCLK_PIN I2S_GPIO_UNUSED

static i2s_chan_handle_t s_tx_handle;

static size_t i2s_tx_q_ovf_count = 0;
static IRAM_ATTR bool i2s_tx_queue_overflow_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *data) {
  i2s_tx_q_ovf_count++;
  return false;
}

void i2s_init(void) {
  i2s_chan_config_t chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  // dma_buffer_size = dma_frame_num * slot_num * data_bit_width / 8 <= 4092
  chan_cfg.dma_desc_num = 2;
  chan_cfg.dma_frame_num = I2S_TX_AUDIO_BUFFER >> 1;

  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &s_tx_handle, NULL));

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_TX_SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                      I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_SCLK_PIN,
              .bclk = I2S_BLK_PIN,
              .ws = I2S_WS_PIN,
              .dout = I2S_DATA_OUT_PIN,
              .din = I2S_DATA_IN_PIN,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_tx_handle, &std_cfg));

  i2s_event_callbacks_t cbs = {
      .on_recv = NULL,
      .on_recv_q_ovf = NULL,
      .on_sent = NULL,
      .on_send_q_ovf = i2s_tx_queue_overflow_callback,
  };
  ESP_ERROR_CHECK(i2s_channel_register_event_callback(s_tx_handle, &cbs, NULL));
  ESP_ERROR_CHECK(i2s_channel_enable(s_tx_handle));
}

void i2s_play_wav(const void *data, size_t bytes) {
  size_t wrote_bytes = 0;
  i2s_channel_write(s_tx_handle, data, bytes, &wrote_bytes, portMAX_DELAY);
  ESP_LOGD(TAG, "wrote bytes=%d/%d", wrote_bytes, bytes);
}

void i2s_release(void) {
  ESP_ERROR_CHECK(i2s_channel_disable(s_tx_handle));
  ESP_ERROR_CHECK(i2s_del_channel(s_tx_handle));
}