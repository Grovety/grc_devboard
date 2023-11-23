#include "KWS.hpp"
#include "Common.hpp"
#include "Event.hpp"
#include "Status.hpp"

#include "kws.h"
#include "mfcc_preprocessor.h"
#include "mic_fragment_reader.h"

#define MIC_NOISY_ENV_THRESHOLD 350
#define MIC_BAD_ENV_THRESHOLD 700

static MicFragmentReader s_mic;
static MfccPreprocessor s_pp;

static const char *TAG = "KWS";

static void kws_event_task(void *pvParameters) {
  LOGD(TAG, "Init MIC listener...");
  int errors = 0;
  s_mic.Setup();
  if (errors += s_mic.Errors() + s_pp.Errors()) {
    LOGE(TAG, "start errors. stop");
    vTaskDelete(NULL);
  }
  xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_INIT_MSK);

  char result[32];
  float *buffer = NULL;
  for (;;) {
    size_t len = 0;
    xEventGroupWaitBits(xStatusEventGroup,
                        STATUS_CMD_WAIT_MSK | STATUS_SYSTEM_SUSPENDED_MSK,
                        pdFALSE, pdFALSE, portMAX_DELAY);
    xEventGroupWaitBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK, pdTRUE,
                        pdFALSE, portMAX_DELAY);
    if (!s_mic.Collect(nullptr, nullptr, 0)) {
      const auto env = s_mic.GetEnvironment();
      if (env > MIC_BAD_ENV_THRESHOLD) {
        xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_BAD_ENV_MSK);
      } else if (env > MIC_NOISY_ENV_THRESHOLD) {
        xEventGroupClearBits(xStatusEventGroup, STATUS_MIC_BAD_ENV_MSK);
        xEventGroupSetBits(xStatusEventGroup, STATUS_MIC_NOISY_ENV_MSK);
      } else {
        xEventGroupClearBits(xStatusEventGroup, STATUS_MIC_ENV_BITS_MSK);
      }
    } else {
      len += s_mic.Collect(&s_pp, (void **)(&buffer), 1);
    }

    if (len > 0 && len < 1300) {
      xEventGroupClearBits(xStatusEventGroup, STATUS_MIC_ENV_BITS_MSK);
      xEventGroupSetBits(xStatusEventGroup, STATUS_KWS_BUSY_MSK);
      recognize_word(buffer, len, result);
      xEventGroupClearBits(xStatusEventGroup, STATUS_KWS_BUSY_MSK);
      LOGI(TAG, "result=%s", result);

      if (strcmp(result, "_silence_") != 0 &&
          strcmp(result, "_unknown_") != 0) {
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
        } else if (strcmp(result, "go") == 0) {
          sendEvent(eEvent::CMD_GO);
        } else if (strcmp(result, "up") == 0) {
          sendEvent(eEvent::CMD_UP);
        } else if (strcmp(result, "sheila") == 0) {
          sendEvent(eEvent::CMD_SHEILA);
        } else {
          sendEvent(eEvent::CMD_UNKNOWN);
        }
        s_mic.EnvironmentArrayRightshift();
      } else {
        sendEvent(eEvent::CMD_UNKNOWN);
      }

      delete[] buffer;
      buffer = NULL;
    }
    xEventGroupSetBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK);
  }
}

void initKWS() {
  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(kws_event_task, "kws_event_task",
                  configMINIMAL_STACK_SIZE + 1024 * 4, NULL, 1, NULL);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating KWS event task");
    vTaskDelete(xHandle);
  }
}