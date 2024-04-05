#include "WavPlayer.hpp"
#include "I2sTx.hpp"
#include "Types.hpp"
#include "i2s_rx_slot.h"

#include "driver/gpio.h"

#define GAIN_PIN GPIO_NUM_42
#define SDMODE_PIN GPIO_NUM_9

static const char *TAG = "WavPlayer";

EventGroupHandle_t xWavPlayerEventGroup;
QueueHandle_t xWavPlayerQueue;

static void wp_task(void *pvParameters) {
  for (;;) {
    Sample_t sample;
    xQueuePeek(xWavPlayerQueue, &sample, portMAX_DELAY);
    if (xSemaphoreTake(xMicSema, portMAX_DELAY) == pdPASS) {
      xEventGroupClearBits(xWavPlayerEventGroup, WAV_PLAYER_STOP_MSK);
      while (xQueueReceive(xWavPlayerQueue, &sample, 0) == pdPASS) {
        if (sample.data) {
          i2s_play_wav(sample.data, sample.bytes);
        } else {
          LOGE(TAG, "wav data is not allocated");
        }
      }
      xSemaphoreGive(xMicSema);
      xEventGroupSetBits(xWavPlayerEventGroup, WAV_PLAYER_STOP_MSK);
    }
  }
}

void initWavPlayer() {
  LOGD(TAG, "Setting up i2s");
  i2s_init();

  gpio_set_direction(SDMODE_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(SDMODE_PIN, 1);

  xWavPlayerQueue = xQueueCreate(100, sizeof(Sample_t));
  if (xWavPlayerQueue == NULL) {
    LOGE(TAG, "Error creating wav queue");
  }

  xWavPlayerEventGroup = xEventGroupCreate();
  if (xWavPlayerEventGroup == NULL) {
    LOGE(TAG, "Error creating xWavPlayerEventGroup");
  }
  xEventGroupSetBits(xWavPlayerEventGroup, WAV_PLAYER_STOP_MSK);

  TaskHandle_t xHandle = NULL;
  auto xReturned = xTaskCreate(
      wp_task, "wp_task", configMINIMAL_STACK_SIZE + 1024, NULL, 3, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating wp_task");
    vTaskDelete(xHandle);
    i2s_release();
  }
}
