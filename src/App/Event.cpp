#include "Event.hpp"
#include "Common.hpp"
#include "IButton.hpp"

enum eButtonState { DOWN, UP };

#define BUTTON_HOLD_DURATION_MS 800

QueueHandle_t xEventQueue;

constexpr char TAG[] = "Event";

void sendEvent(eEvent ev) { xQueueSend(xEventQueue, &ev, pdMS_TO_TICKS(200)); }

static void button_event_task(void *pvParameters) {
  auto *p_button = static_cast<IButton *>(pvParameters);
  eButtonState bstate = eButtonState::UP;
  unsigned start_time = 0;
  bool pending_hold = false;

  for (;;) {
    if (bstate == eButtonState::UP) {
      if (p_button->isPressed()) {
        bstate = eButtonState::DOWN;
        start_time = getTimeMS();
        sendEvent(eEvent::BUTTON_TO_DOWN);
      }
    } else if (bstate == eButtonState::DOWN) {
      if (!pending_hold) {
        if (getTimeMS() > start_time + BUTTON_HOLD_DURATION_MS) {
          pending_hold = true;
          sendEvent(eEvent::BUTTON_HOLD);
        } else if (!p_button->isPressed()) {
          bstate = eButtonState::UP;
          sendEvent(eEvent::BUTTON_TO_UP);
          sendEvent(eEvent::BUTTON_CLICK);
        }
      } else if (!p_button->isPressed()) {
        pending_hold = false;
        bstate = eButtonState::UP;
        sendEvent(eEvent::BUTTON_TO_UP);
      }
    }
  }
}

void initEventsGenerator(IButton *p_button) {
  xEventQueue = xQueueCreate(1, sizeof(eEvent));
  if (xEventQueue == NULL) {
    LOGE(TAG, "Error creating event queue");
  }

  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(button_event_task, "button_event_task",
                  configMINIMAL_STACK_SIZE + 1024, p_button, 1, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating button event task");
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
  case eEvent::CMD_GO:
    return "g";
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
  default:
    return 0;
  }
}