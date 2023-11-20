#include "App.hpp"
#include "State.hpp"

#include "Accelerometer.hpp"
#include "Button.hpp"
#include "DisplaySTDOUT.hpp"
#include "KWS.hpp"
#include "Lcd.hpp"
#include "Led.hpp"
#include "NVStorage.hpp"

#include "string.h"

#define TITLE "GRC"
#define VERSION "1.0"

constexpr char TAG[] = "App";

App::App() : context_({"main"}) {
  context_.reserve(32);
  transition(&Common::States::init);
}

void App::transition(State *target_state) {
  if (current_state_.ptr != nullptr) {
    current_state_.ptr->exitAction(this);
  }
  if (target_state != nullptr) {
    target_state->enterAction(this);
  }
  current_state_ = {target_state, getTimeMS()};
}

bool App::check_timeout(size_t timeout) const {
  if ((timeout != 0) && (timeout + current_state_.enter_time < getTimeMS())) {
    return true;
  }
  return false;
}

void App::run() {
  const unsigned event_wait_ms = 100;
  const TickType_t xTicksToWait = event_wait_ms / portTICK_PERIOD_MS;
  Common::Suspended sus_state(nullptr);
  unsigned no_event_time = getTimeMS();

  while (1) {
    if (current_state_.ptr == nullptr) {
      break;
    }
    eEvent ev;
    if (xQueueReceive(xEventQueue, &ev, xTicksToWait) == pdPASS) {
      no_event_time = getTimeMS();
      p_display->print_status(decode_cmd_event(ev));
      current_state_.ptr->handleEvent(this, ev);
    } else {
      current_state_.ptr->update(this);
      const auto xBits = xEventGroupGetBits(xStatusEventGroup);
      if (!(xBits & STATUS_SYSTEM_SUSPENDED_MSK) && !(xBits & STATUS_SYSTEM_BUSY_MSK)) {
        if (no_event_time + SUSPEND_AFTER_MS < getTimeMS()) {
          sus_state = Common::Suspended(current_state_.ptr);
          transition(&sus_state);
        }
      }
    }
  }
}

namespace Common {
struct Init : State {
  std::unique_ptr<ISensor> p_sensor;
  std::unique_ptr<IButton> p_button;
  std::unique_ptr<ILed> p_led;

  void enterAction(App *app) override final {
    app->p_storage = std::make_unique<NVStorage>();
    p_led = std::make_unique<Led>();
    initStatusMonitor(p_led.get());

    p_button = std::make_unique<Button>();
    initEventsGenerator(p_button.get());
    initKWS();

    p_sensor = std::make_unique<Accelerometer>();
    float accel_data[3] = {0};
    if (p_sensor->open()) {
      xEventGroupSetBits(xStatusEventGroup, STATUS_SENSOR_INIT_MSK);
      p_sensor->getData(accel_data, _countof(accel_data));
      initListener(p_sensor.get());
    } else {
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    }

#ifdef NO_LCD
    app->p_display = std::make_unique<DisplaySTDOUT>();
#else
    app->p_display = std::make_unique<Lcd>(
        accel_data[Accelerometer::Acl_y] > 0.03 ? Lcd::Rotation::PORTRAIT
                                                : Lcd::Rotation::UPSIDE_DOWN);
#endif // NO_LCD

    app->set_ctx(TITLE " " VERSION);
    app->p_display->print_header("%s", app->get_ctx());
    app->p_display->send();
  }
  void update(App *app) override final {
    xEventGroupWaitBits(xStatusEventGroup, STATUS_INIT_BITS_MSK, pdFALSE,
                        pdTRUE, pdMS_TO_TICKS(3000));
    app->transition(&States::select_scenario_menu);
  }
};

void Menu::drawMenuItems(App *app) const {
  for (unsigned i = 0; i < items_.size(); ++i) {
    const unsigned num = getCurrentItem();
    if (i == num) {
      app->p_display->print_string("%d.%s<<", num + 1, items_[num].name);
    } else {
      app->p_display->print_string("%d.%s", i + 1, items_[i].name);
    }
  }
}
void Menu::drawMenu(App *app) const {
  app->p_display->print_header("%s", app->get_ctx());
  drawMenuItems(app);
  app->p_display->send();
}
unsigned Menu::getCurrentItem() const { return current_item_ % items_.size(); }
void Menu::enterAction(App *app) {
  drawMenu(app);
  xEventGroupSetBits(xStatusEventGroup, STATUS_CMD_WAIT_MSK);
}
void Menu::exitAction(App *app) {
  xEventGroupClearBits(xStatusEventGroup, STATUS_CMD_WAIT_MSK);
  current_item_ = 0;
}
void Menu::handleEvent(App *app, eEvent ev) {
  switch (ev) {
  case eEvent::BUTTON_CLICK:
    current_item_++;
    drawMenu(app);
    break;
  case eEvent::BUTTON_HOLD:
    app->transition(items_[getCurrentItem()].target_state);
    break;
  case eEvent::CMD_ONE:
  case eEvent::CMD_TWO:
  case eEvent::CMD_THREE: {
    const unsigned num = cmd_event_to_number(ev);
    if (num > 0 && num <= items_.size()) {
      app->transition(items_[num - 1].target_state);
    } else {
      drawMenu(app);
    }
  } break;
  case eEvent::CMD_UP:
    if (back_state_ != nullptr) {
      app->transition(back_state_);
    } else {
      drawMenu(app);
    }
    break;
  case eEvent::BUTTON_TO_UP:
  case eEvent::BUTTON_TO_DOWN:
    break;
  default:
    drawMenu(app);
    break;
  }
}

void ConfirmDialog::drawMenuItems(App *app) const {
  if (getCurrentItem() == 0) {
    app->p_display->print_string("%s<< %s", items_[0].name, items_[1].name);
  } else {
    app->p_display->print_string("%s   %s<<", items_[0].name, items_[1].name);
  }
}
void ConfirmDialog::handleEvent(App *app, eEvent ev) {
  switch (ev) {
  case eEvent::BUTTON_CLICK:
    current_item_++;
    drawMenu(app);
    break;
  case eEvent::BUTTON_HOLD:
    app->transition(items_[getCurrentItem()].target_state);
    break;
  case eEvent::CMD_YES:
    app->transition(items_[0].target_state);
    break;
  case eEvent::CMD_NO:
    app->transition(items_[1].target_state);
    break;
  case eEvent::CMD_UP:
    if (back_state_ != nullptr) {
      app->transition(back_state_);
    } else {
      drawMenu(app);
    }
    break;
  case eEvent::BUTTON_TO_UP:
  case eEvent::BUTTON_TO_DOWN:
    break;
  default:
    drawMenu(app);
    break;
  }
}

void BackTo::update(App *app) {
  app->pop_ctx();
  app->transition(target_state_);
}

void Listen::enterAction(App *app) {
  updateDisplay(app);
  xEventGroupSetBits(xStatusEventGroup, STATUS_CMD_WAIT_MSK);
}
void Listen::exitAction(App *app) {
  xEventGroupClearBits(xStatusEventGroup, STATUS_CMD_WAIT_MSK);
}

void Suspended::enterAction(App *app) {
  LOGI(TAG, "Entering suspended state");
  app->p_display->clear();
  app->p_display->send();
  xEventGroupSetBits(xStatusEventGroup, STATUS_SYSTEM_SUSPENDED_MSK);
}
void Suspended::exitAction(App *app) {
  LOGI(TAG, "Exiting suspended state");
  xEventGroupClearBits(xStatusEventGroup, STATUS_SYSTEM_SUSPENDED_MSK);
}
void Suspended::handleEvent(App *app, eEvent ev) {
  switch (ev) {
  case eEvent::BUTTON_CLICK:
  case eEvent::BUTTON_HOLD:
  case eEvent::CMD_SHEILA:
    app->transition(back_state_);
    break;
  default:
    break;
  }
}

State &States::init = *new Init;
State &States::init_gesture_scenario = *new InitGestureScenario;
State &States::init_rhythm_scenario = *new InitRhythmScenario;
State &States::init_vibro_scenario = *new InitVibroScenario;
State &States::select_scenario_menu = *new Menu(
    {
        {"Gesture", &States::init_gesture_scenario},
        {"Rhythm", &States::init_rhythm_scenario},
        {"Vibro", &States::init_vibro_scenario},
    },
    nullptr);

int process_grc_replies(eGrcReplyId id) {
  GrcReply_t reply;
  while (xQueueReceive(GrcControllerQueues.xTxQueue, &reply,
                       pdMS_TO_TICKS(10000)) == pdPASS) {
    if (reply.id == id) {
      return reply.data;
    }
  }
  LOGE(TAG, "Timeout waiting for a reply from grc");
  return -1;
}
} // namespace Common
