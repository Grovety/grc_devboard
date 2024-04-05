#include "KWS.hpp"
#include "Common.hpp"
#include "Event.hpp"
#include "Status.hpp"

#include "kws_preprocessor.h"
#include "mic_reader.h"

static const char *TAG = "KWS";

extern const unsigned char *kws_cmd_model_ptr;
extern const char *kws_cmd_labels[];
extern unsigned int kws_cmd_labels_num;

static void kws_event_task(void *pvParameters) {
  char result[32];
  for (;;) {
    size_t req_words = 0;
    xQueuePeek(xKWSRequestQueue, &req_words, portMAX_DELAY);
    if (xSemaphoreTake(xKWSSema, pdMS_TO_TICKS(50))) {
      int category;
      if (xQueueReceive(xKWSResultQueue, &category, pdMS_TO_TICKS(50)) == pdPASS) {
        kws_get_category(category, result, sizeof(result));
        if (strcmp(result, "yes") == 0) {
          sendEvent(eEvent::CMD_YES);
        } else if (strcmp(result, "no") == 0) {
          sendEvent(eEvent::CMD_NO);
        } else if (strcmp(result, "one") == 0) {
          sendEvent(eEvent::CMD_ONE);
        } else if (strcmp(result, "two") == 0) {
          sendEvent(eEvent::CMD_TWO);
        } else if (strcmp(result, "three") == 0) {
          sendEvent(eEvent::CMD_THREE);
        } else if (strcmp(result, "four") == 0) {
          sendEvent(eEvent::CMD_FOUR);
        } else if (strcmp(result, "up") == 0) {
          sendEvent(eEvent::CMD_UP);
        } else if (strcmp(result, "sheila") == 0) {
          sendEvent(eEvent::CMD_SHEILA);
        } else {
          sendEvent(eEvent::CMD_UNKNOWN);
          kws_req_word(1);
        }
      }
      xSemaphoreGive(xKWSSema);
    } else {
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

bool initKWS() {
  int errors = !kws_model_init_op_resolver();
  errors += !kws_model_init(kws_cmd_model_ptr, kws_cmd_labels, kws_cmd_labels_num);
  if (errors) {
    return false;
  }
  kws_preprocessor_init();
  errors = !mic_reader_init();
  if (errors) {
    return false;
  }

  auto xReturned =
      xTaskCreate(kws_event_task, "kws_event_task",
                  configMINIMAL_STACK_SIZE + 1024 * 4, NULL, tskIDLE_PRIORITY, NULL);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating KWS event task");
    return false;
  }

  xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_INIT_MSK);
  return true;
}