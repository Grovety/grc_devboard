#include "Event.hpp"
#include "Common.hpp"
#include "Button.hpp"

enum eButtonState { DOWN, UP };

#define BOOT_BUTTON_PIN 0
#define SW3_BUTTON_PIN  1
#define SW4_BUTTON_PIN  2

#define BUTTON_HOLD_DURATION_MS 800

TimerHandle_t xTimer;
QueueHandle_t xEventQueue;

static constexpr char TAG[] = "Event";

void sendEvent(eEvent ev) { xQueueSend(xEventQueue, &ev, 0); }

static void boot_button_event_task(void *pvParameters) {
  Button button(BOOT_BUTTON_PIN);
  eButtonState bstate = eButtonState::UP;
  unsigned start_time = 0;
  bool pending_hold = false;

  for (;;) {
    if (bstate == eButtonState::UP) {
      if (button.isPressed()) {
        bstate = eButtonState::DOWN;
        start_time = getTimeMS();
        sendEvent(eEvent::BUTTON_TO_DOWN);
      }
    } else if (bstate == eButtonState::DOWN) {
      if (!pending_hold) {
        if (getTimeMS() > start_time + BUTTON_HOLD_DURATION_MS) {
          pending_hold = true;
          sendEvent(eEvent::BUTTON_HOLD);
        } else if (!button.isPressed()) {
          bstate = eButtonState::UP;
          sendEvent(eEvent::BUTTON_TO_UP);
          sendEvent(eEvent::BUTTON_CLICK);
        }
      } else if (!button.isPressed()) {
        pending_hold = false;
        bstate = eButtonState::UP;
        sendEvent(eEvent::BUTTON_TO_UP);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void sw3_button_event_task(void *pvParameters) {
  Button button(SW3_BUTTON_PIN);
  eButtonState bstate = eButtonState::UP;
  unsigned start_time = 0;
  bool pending_hold = false;

  for (;;) {
    if (bstate == eButtonState::UP) {
      if (button.isPressed()) {
        bstate = eButtonState::DOWN;
        start_time = getTimeMS();
      }
    } else if (bstate == eButtonState::DOWN) {
      if (!pending_hold) {
        if (getTimeMS() > start_time + BUTTON_HOLD_DURATION_MS) {
          pending_hold = true;
          sendEvent(eEvent::CMD_SLEEP);
        } else if (!button.isPressed()) {
          bstate = eButtonState::UP;
          sendEvent(eEvent::CMD_UP);
        }
      } else if (!button.isPressed()) {
        pending_hold = false;
        bstate = eButtonState::UP;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void sw4_button_event_task(void *pvParameters) {
  Button button(SW4_BUTTON_PIN);
  eButtonState bstate = eButtonState::UP;
  unsigned start_time = 0;
  bool pending_hold = false;

  for (;;) {
    if (bstate == eButtonState::UP) {
      if (button.isPressed()) {
        bstate = eButtonState::DOWN;
        start_time = getTimeMS();
        sendEvent(eEvent::BUTTON_TO_DOWN);
      }
    } else if (bstate == eButtonState::DOWN) {
      if (!pending_hold) {
        if (getTimeMS() > start_time + BUTTON_HOLD_DURATION_MS) {
          pending_hold = true;
          sendEvent(eEvent::BUTTON_HOLD);
        } else if (!button.isPressed()) {
          bstate = eButtonState::UP;
          sendEvent(eEvent::BUTTON_TO_UP);
          sendEvent(eEvent::BUTTON_CLICK);
        }
      } else if (!button.isPressed()) {
        pending_hold = false;
        bstate = eButtonState::UP;
        sendEvent(eEvent::BUTTON_TO_UP);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void vTimerCallback(TimerHandle_t xTimer) {
  sendEvent(eEvent::TIMEOUT);
}

void initEventsGenerator() {
  xEventQueue = xQueueCreate(5, sizeof(eEvent));
  if (xEventQueue == NULL) {
    LOGE(TAG, "Error creating event queue");
  }
  xTimer = xTimerCreate("Timer", 0, pdFALSE, NULL, vTimerCallback);
  if (xTimer == NULL) {
    LOGE(TAG, "Error creating xTimer");
  }

  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(boot_button_event_task, "boot_button_event_task",
                  configMINIMAL_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating boot button event task");
    vTaskDelete(xHandle);
  }
  xReturned =
      xTaskCreate(sw3_button_event_task, "sw3_button_event_task",
                  configMINIMAL_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating sw3 button event task");
    vTaskDelete(xHandle);
  }
  xReturned =
      xTaskCreate(sw4_button_event_task, "sw4_button_event_task",
                  configMINIMAL_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating sw3 button event task");
    vTaskDelete(xHandle);
  }
}

const char *decode_cmd_event(eEvent ev) {
  switch (ev) {
  case eEvent::CMD_YES:
    return "y";
  case eEvent::CMD_NO:
    return "n";
  case eEvent::CMD_ONE:
    return "1";
  case eEvent::CMD_TWO:
    return "2";
  case eEvent::CMD_THREE:
    return "3";
  case eEvent::CMD_FOUR:
    return "4";
  case eEvent::CMD_UP:
    return "u";
  case eEvent::CMD_SHEILA:
    return "s";
  case eEvent::CMD_UNKNOWN:
    return "-";
  default:
    return "";
  }
}

unsigned cmd_event_to_number(eEvent ev) {
  switch (ev) {
  case eEvent::CMD_ONE:
    return 1;
  case eEvent::CMD_TWO:
    return 2;
  case eEvent::CMD_THREE:
    return 3;
  case eEvent::CMD_FOUR:
    return 4;
  default:
    return 0;
  }
}