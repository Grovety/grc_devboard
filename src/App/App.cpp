#include "App.hpp"
#include "State.hpp"

#include "Accelerometer.hpp"
#include "Button.hpp"
#include "DisplaySTDOUT.hpp"
#include "KWS.hpp"
#include "Lcd.hpp"
#include "Led.hpp"
#include "NVStorage.hpp"

#include "kws_preprocessor.h"

#include "string.h"

#define TITLE "GRC"
#define VERSION "1.1"

static constexpr char TAG[] = "App";

App::App() : context_({"main"}) {
  transition_queue_ = xQueueCreate(1, sizeof(State*));
  if (transition_queue_ == NULL) {
    LOGE(TAG, "Error creating transition queue");
  }
  context_.reserve(32);
  p_storage = std::make_unique<NVStorage>();
  p_led = std::make_unique<Led>();
  initStatusMonitor(p_led.get());

  initEventsGenerator();

  p_sensor = std::make_unique<Accelerometer>();
  float accel_data[3] = {0};
  const bool accel_init_res = p_sensor->open();
  if (accel_init_res) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_SENSOR_INIT_MSK);
    p_sensor->getData(accel_data, _countof(accel_data));
    initListener(p_sensor.get());
  }

#ifdef NO_LCD
  p_display = std::make_unique<DisplaySTDOUT>();
#else
  p_display = std::make_unique<Lcd>(
      accel_data[Accelerometer::Acl_y] > 0.03 ? Lcd::Rotation::PORTRAIT
                                              : Lcd::Rotation::UPSIDE_DOWN);
#endif // NO_LCD

  set_ctx(TITLE " " VERSION);
  p_display->print_header("%s", get_ctx());
  p_display->send();

  const bool kws_init_res = initKWS();

  if (!kws_init_res || !accel_init_res) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    p_display->print_header("%s", get_ctx());
    if (!kws_init_res) {
      p_display->print_string("Mic init err");
    }
    if (!accel_init_res) {
      p_display->print_string("Accel init err");
    }
    p_display->send();
  }

  xEventGroupWaitBits(xStatusEventGroup, STATUS_INIT_BITS_MSK, pdFALSE,
                      pdTRUE, portMAX_DELAY);
  initWavPlayer();
  transition(Common::getMainMenu());
}

void App::transition(State *target_state) {
  xQueueSend(transition_queue_, &target_state, portMAX_DELAY);
}

void App::do_transition(State *target_state) {
  if (current_state_ != nullptr) {
    current_state_->exitAction(this);
    delete current_state_;
    current_state_ = nullptr;
  }
  if (target_state != nullptr) {
    target_state->enterAction(this);
  }
  current_state_ = target_state;
  xQueueReset(xEventQueue);
}

void App::run() {
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  size_t no_event_time = getTimeMS();

  for (;;) {
    State *target_state = nullptr;
    if (xQueueReceive(transition_queue_, &target_state, 0) == pdPASS) {
      do_transition(target_state);
    }
    if (current_state_ == nullptr) {
      return;
    }
    eEvent ev;
    if (xQueueReceive(xEventQueue, &ev, xTicksToWait) == pdPASS) {
      if (ev == eEvent::CMD_SLEEP) {
        const auto xBits = xEventGroupGetBits(xStatusEventGroup);
        if (!(xBits & STATUS_SYSTEM_SUSPENDED_MSK)) {
          transition(new Common::Suspended(current_state_->clone()));
        }
        continue;
      }
      no_event_time = getTimeMS();
      p_display->print_status(decode_cmd_event(ev));
      current_state_->handleEvent(this, ev);
    } else {
      current_state_->update(this);
      const auto xBits = xEventGroupGetBits(xStatusEventGroup);
      if (!(xBits & STATUS_SYSTEM_SUSPENDED_MSK) &&
          !(xBits & STATUS_SYSTEM_BUSY_MSK)) {
        if (no_event_time + SUSPEND_AFTER_MS < getTimeMS()) {
          transition(new Common::Suspended(current_state_->clone()));
        }
      }
    }
  }
}

namespace Common {
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
  void Menu::enterAction(App *app) {
    VoiceMsgPlay(v_msg_id_);
    drawMenu(app);
    kws_req_word(1);
  }
  void Menu::handleEvent(App *app, eEvent ev) {
    switch (ev) {
    case eEvent::BUTTON_CLICK:
      current_item_++;
      drawMenu(app);
      break;
    case eEvent::BUTTON_HOLD:
      VoiceMsgStop();
      kws_req_cancel();
      items_[getCurrentItem()].action(app);
      break;
    case eEvent::CMD_ONE:
    case eEvent::CMD_TWO:
    case eEvent::CMD_THREE:
    case eEvent::CMD_FOUR: {
      const unsigned num = cmd_event_to_number(ev);
      if (num > 0 && num <= items_.size()) {
        items_[num - 1].action(app);
      } else {
        drawMenu(app);
        if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
          kws_req_word(1);
        }
      }
    } break;
    case eEvent::CMD_UP:
      if (back_.action != nullptr) {
        back_.action(app);
      } else {
        drawMenu(app);
        if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
          kws_req_word(1);
        }
      }
      break;
    case eEvent::CMD_YES:
    case eEvent::CMD_NO:
    case eEvent::CMD_SHEILA:
    case eEvent::CMD_UNKNOWN:
      drawMenu(app);
      if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
        kws_req_word(1);
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
  State* Menu::clone() {
    return new Menu(*this);
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
      VoiceMsgStop();
      kws_req_cancel();
      items_[getCurrentItem()].action(app);
      break;
    case eEvent::CMD_YES:
      items_[0].action(app);
      break;
    case eEvent::CMD_NO:
      items_[1].action(app);
      break;
    case eEvent::CMD_UP:
      if (back_.action != nullptr) {
        back_.action(app);
      } else {
        drawMenu(app);
      }
      break;
    case eEvent::CMD_ONE:
    case eEvent::CMD_TWO:
    case eEvent::CMD_THREE:
    case eEvent::CMD_FOUR:
    case eEvent::CMD_SHEILA:
    case eEvent::CMD_UNKNOWN:
      drawMenu(app);
      if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
        kws_req_word(1);
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
  State* ConfirmDialog::clone() {
    return new ConfirmDialog(*this);
  }

  void Suspended::enterAction(App *app) {
    LOGD(TAG, "Entering suspended state");
    app->p_display->clear();
    app->p_display->send();
    xEventGroupSetBits(xStatusEventGroup, STATUS_SYSTEM_SUSPENDED_MSK);
    kws_req_word(1);
  }
  void Suspended::exitAction(App *app) {
    LOGD(TAG, "Exiting suspended state");
    xEventGroupClearBits(xStatusEventGroup, STATUS_SYSTEM_SUSPENDED_MSK);
  }
  void Suspended::handleEvent(App *app, eEvent ev) {
    switch (ev) {
    case eEvent::BUTTON_CLICK:
    case eEvent::BUTTON_HOLD:
      kws_req_cancel();
      app->transition(back_state_);
      break;
    case eEvent::CMD_SHEILA:
      app->transition(back_state_);
      break;
    case eEvent::CMD_YES:
    case eEvent::CMD_NO:
    case eEvent::CMD_ONE:
    case eEvent::CMD_TWO:
    case eEvent::CMD_THREE:
    case eEvent::CMD_FOUR:
    case eEvent::CMD_UP:
    case eEvent::CMD_UNKNOWN:
      if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
        kws_req_word(1);
      }
      break;
    default:
      break;
    }
  }

  State* getMainMenu() {
    return new Common::Menu(
        {
            {"Gesture", &Common::initGestureScenario},
            {"Rhythm", &Common::initRhythmScenario},
            {"Vibro", &Common::initVibroScenario},
            {"VoicePIN", &Common::initVoicePINcodeScenario},
        },
        {"Back", nullptr}, 1);
  }
} // namespace Common
