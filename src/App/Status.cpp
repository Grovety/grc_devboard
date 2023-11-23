#include "Status.hpp"
#include "Common.hpp"
#include "ILed.hpp"

constexpr char TAG[] = "Status";

EventGroupHandle_t xStatusEventGroup;

static void status_monitor_task(void *pvParameters) {
  auto *p_led = static_cast<ILed *>(pvParameters);
  const TickType_t xTicks = 50 / portTICK_PERIOD_MS;
  TickType_t xEnterTime[GET_BIT_POS(STATUS_ALL_BITS_MSK) + 1] = {0};

  auto blinker = [p_led, xTicks, &xEnterTime](
                     EventBits_t mask, TickType_t timeout, ILed::Colour colour,
                     unsigned hold_time, unsigned led_num = -1,
                     ILed::Brightness b = ILed::Brightness::_50) mutable {
    auto &time = xEnterTime[GET_BIT_POS(mask)];
    time += xTicks;
    if (time > timeout) {
      p_led->set(colour, led_num, b);
      delayMS(hold_time);
      time = 0;
    } else {
      p_led->set(ILed::Black, led_num);
    }
  };

  for (;;) {
    auto xBits = xEventGroupWaitBits(xStatusEventGroup, STATUS_EVENT_BITS_MSK,
                                     pdTRUE, pdFALSE, xTicks);
    if (xBits & STATUS_EVENT_BITS_MSK) {
      if (xBits & STATUS_EVENT_GOOD_MSK) {
        p_led->set(ILed::Green);
        delayMS(1500);
      } else if (xBits & STATUS_EVENT_BAD_MSK) {
        p_led->set(ILed::Red);
        delayMS(1500);
      }
      p_led->set(ILed::Black);
    } else {
      xBits = xEventGroupGetBits(xStatusEventGroup);

      if (xBits & STATUS_SENSOR_LISTENER_BUSY_MSK) {
        p_led->set(ILed::White);
      } else if (xBits & STATUS_KWS_BUSY_MSK) {
        blinker(STATUS_KWS_BUSY_MSK, 200, ILed::Yellow, 100);
      } else if (xBits & STATUS_CMD_WAIT_MSK) {
        if (xBits & STATUS_MIC_BAD_ENV_MSK) {
          p_led->set(ILed::Black);
        } else if (xBits & STATUS_MIC_NOISY_ENV_MSK) {
          p_led->set(ILed::Blue, -1, ILed::Brightness::_25);
        } else {
          p_led->set(ILed::Blue);
        }
      } else if (xBits & STATUS_SYSTEM_SUSPENDED_MSK) {
        blinker(STATUS_SYSTEM_SUSPENDED_MSK, 5000, ILed::Cyan, 200, -1,
                ILed::Brightness::_25);
      } else {
        p_led->set(ILed::Black);
      }
    }
  }
}

void initStatusMonitor(ILed *p_led) {
  xStatusEventGroup = xEventGroupCreate();
  if (xStatusEventGroup == NULL) {
    LOGE(TAG, "Error creating xStatusEventGroup");
  }
  xEventGroupSetBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK);

  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(status_monitor_task, "status_monitor_task",
                  configMINIMAL_STACK_SIZE + 512, p_led, 2, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating task");
    vTaskDelete(xHandle);
  }
}