#include "App.hpp"
#include "State.hpp"
#include "VibroGrc.hpp"
#include "string.h"

extern HP g_HP_Ventilator;
constexpr char TAG[] = "VibroScenario";

namespace VibroScenario {
struct States {
  static State &select_ad;
  static State &select_clear_dataset;
  static State &menu;

  static State &back_to_menu;

  static State &infer;
  static State &listen;
  static State &print_category;
  static State &process_no_match;

  static State &saved;
  static State &overwrite_sample;
  static State &overwrite_dialog;

  static State &clear_dataset_dialog;
  static State &clear_dataset;
};

static int s_category = -1;
static int s_cats_qty = -1;

struct Listen : Common::Listen {
  using Common::Listen::Listen;
  void updateDisplay(App *app) override final {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Press User to go");
    app->p_display->send();
  }
  void handleEvent(App *app, eEvent ev) override final {
    switch (ev) {
    case eEvent::CMD_GO:
    case eEvent::BUTTON_CLICK: {
      app->transition(forward_state_);
    } break;
    case eEvent::CMD_UP:
      if (back_state_ != nullptr) {
        app->transition(back_state_);
      } else {
        updateDisplay(app);
      }
      break;
    default:
      updateDisplay(app);
      break;
    }
  }
};

struct Infer : State {
  void enterAction(App *app) override final {
    xEventGroupSetBits(xStatusEventGroup, STATUS_CMD_WAIT_MSK | STATUS_SYSTEM_BUSY_MSK);
  }
  void handleEvent(App *app, eEvent ev) override final {
    switch (ev) {
    case eEvent::CMD_UP:
      app->transition(&States::back_to_menu);
      break;
    default:
      break;
    }
  }
  void update(App *app) override final {
    SignalData_t data;
    data.num_rows = SIGNAL_COMPS_NUM;
    data.num_cols = SIGNAL_SAMPLES_NUM;
    data.sync_time_ms = SIGNAL_PERIOD_MS;
    data.buffer = new float[data.num_rows * data.num_cols];
    xQueueSend(ListenerQueues.xRxQueue, &data, portMAX_DELAY);
    delayMS(400);
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Detecting...");
    app->p_display->send();
    if (xQueueReceive(ListenerQueues.xTxQueue, &data, portMAX_DELAY) == pdPASS) {
      LOGD(TAG, "Done: num_cols=%d", data.num_cols);
      GrcEvent_t load_ev{eGrcEventId::LOAD_SIGNAL, new SignalData_t(data)};
      xQueueSend(GrcControllerQueues.xRxQueue, &load_ev, portMAX_DELAY);
      GrcEvent_t infer_ev{eGrcEventId::INFER, nullptr};
      xQueueSend(GrcControllerQueues.xRxQueue, &infer_ev, portMAX_DELAY);
      s_category = Common::process_grc_replies(eGrcReplyId::CATEGORY);
      if (s_category < 0) {
        app->transition(&States::process_no_match);
      } else {
        app->transition(&States::print_category);
      }
    }
  }
};

struct PrintCategory : State {
  void enterAction(App *app) override final {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Pattern %d", s_category + 1);
    app->p_display->send();
  }
  void handleEvent(App *app, eEvent ev) override final {
    switch (ev) {
    case eEvent::CMD_UP:
      app->transition(&States::back_to_menu);
      break;
    default:
      break;
    }
  }
  void update(App *app) override final {
    if (app->check_timeout(1000)) {
      app->transition(&States::infer);
    }
  }
};

void learn(App *app) {
  SignalData_t data;
  data.num_rows = SIGNAL_COMPS_NUM;
  data.num_cols = SIGNAL_SAMPLES_NUM;
  data.sync_time_ms = SIGNAL_PERIOD_MS;
  data.buffer = new float[data.num_rows * data.num_cols];
  delayMS(2000);
  xQueueSend(ListenerQueues.xRxQueue, &data, portMAX_DELAY);
  if (xQueueReceive(ListenerQueues.xTxQueue, &data, portMAX_DELAY) == pdPASS) {
    GrcEvent_t load_ev{eGrcEventId::LOAD_SIGNAL, new SignalData_t(data)};
    xQueueSend(GrcControllerQueues.xRxQueue, &load_ev, portMAX_DELAY);
    GrcEvent_t train_ev{eGrcEventId::TRAIN,
                        new int(s_cats_qty < MAX_TRAINABLE_CATEGORIES
                                    ? s_cats_qty
                                    : s_cats_qty - 1)};
    xQueueSend(GrcControllerQueues.xRxQueue, &train_ev, portMAX_DELAY);
    if (s_cats_qty + 1 > MAX_TRAINABLE_CATEGORIES) {
      app->transition(&States::overwrite_dialog);
    } else {
      GrcEvent_t save_ev{eGrcEventId::SAVE_WGTS, app->p_storage.get()};
      xQueueSend(GrcControllerQueues.xRxQueue, &save_ev, portMAX_DELAY);
      s_cats_qty++;
      app->transition(&States::saved);
    }
  }
}

struct ProcessNoMatch : State {
  void enterAction(App *app) override final {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Anomaly detected");
    app->p_display->print_string("Learn pattern?");
    app->p_display->print_string(">> Y <<");
    app->p_display->send();
  }
  void handleEvent(App *app, eEvent ev) override final {
    switch (ev) {
    case eEvent::CMD_UP:
      app->transition(&States::back_to_menu);
      break;
    case eEvent::CMD_YES:
    case eEvent::BUTTON_CLICK:
    case eEvent::BUTTON_HOLD:
      learn(app);
      break;
    default:
      break;
    }
  }
  void update(App *app) override final {
    if (app->check_timeout(1000)) {
      app->transition(&States::infer);
    }
  }
};

struct Saved : State {
  void enterAction(App *app) override final {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Pattern %d saved", s_cats_qty);
    app->p_display->send();
  }
  void update(App *app) override final {
    if (app->check_timeout(1000)) {
      app->transition(&States::infer);
    }
  }
};

struct OverwriteDialog : Common::ConfirmDialog {
  using Common::ConfirmDialog::ConfirmDialog;
  void enterAction(App *app) override final {
    Common::ConfirmDialog::enterAction(app);
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
  }
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Memory is full.");
    app->p_display->print_string("Overwrite last?");
    drawMenuItems(app);
    app->p_display->send();
  }
};

struct OverwriteSample : State {
  void update(App *app) override final {
    GrcEvent_t save_ev{eGrcEventId::SAVE_WGTS, app->p_storage.get()};
    xQueueSend(GrcControllerQueues.xRxQueue, &save_ev, portMAX_DELAY);
    app->transition(&States::infer);
  }
};

struct SelectAnomalyDetection : State {
  static constexpr const char *name = "AD";

  void enterAction(App *app) override final { app->set_ctx(name); }
  void update(App *app) override final { app->transition(&States::listen); }
};

struct SelectClearDataset : State {
  static constexpr const char *name = "Clr data";

  void exitAction(App *app) override final { app->set_ctx(name); }
  void update(App *app) override final {
    app->transition(&States::clear_dataset_dialog);
  }
};

struct MainMenu : Common::Menu {
  using Common::Menu::Menu;
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    drawMenuItems(app);
    app->p_display->send();
  }
  void enterAction(App *app) override final {
    if (s_cats_qty == -1) {
      app->p_display->print_header("%s", app->get_ctx());
      app->p_display->send();
      s_cats_qty = Common::process_grc_replies(eGrcReplyId::CATS_QTY);
    }
    Common::Menu::enterAction(app);
  }
};

struct ClearDatasetDialog : Common::ConfirmDialog {
  using Common::ConfirmDialog::ConfirmDialog;
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Are you sure?");
    drawMenuItems(app);
    app->p_display->send();
  }
};

struct ClearDataset : State {
  void enterAction(App *app) override final {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("The dataset");
    app->p_display->print_string("is deleted");
    app->p_display->send();
    GrcEvent_t grc_ev{eGrcEventId::CLEAR_WGTS, app->p_storage.get()};
    xQueueSend(GrcControllerQueues.xRxQueue, &grc_ev, portMAX_DELAY);
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
  }
  void exitAction(App *app) override final {
    xEventGroupClearBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
    s_cats_qty = Common::process_grc_replies(eGrcReplyId::CATS_QTY);
  }
  void update(App *app) override final {
    if (app->check_timeout(1000)) {
      app->transition(&States::back_to_menu);
    }
  }
};

struct BackTo : Common::BackTo {
  using Common::BackTo::BackTo;
  void exitAction(App *app) override final {
    xEventGroupClearBits(xStatusEventGroup, STATUS_SYSTEM_BUSY_MSK);
  }
};

State &States::select_ad = *new SelectAnomalyDetection;
State &States::select_clear_dataset = *new SelectClearDataset;
State &States::menu = *new MainMenu(
    {
        {SelectAnomalyDetection::name, &States::select_ad},
        {SelectClearDataset::name, &States::select_clear_dataset},
    },
    nullptr);
State &States::back_to_menu = *new BackTo(&States::menu);

State &States::saved = *new Saved;
State &States::overwrite_sample = *new OverwriteSample;

State &States::infer = *new Infer;
State &States::listen = *new Listen(&States::infer, &States::back_to_menu);
State &States::print_category = *new PrintCategory;
State &States::process_no_match = *new ProcessNoMatch;
State &States::overwrite_dialog =
    *new OverwriteDialog(&States::overwrite_sample, &States::infer);

State &States::clear_dataset = *new ClearDataset;
State &States::clear_dataset_dialog =
    *new ClearDatasetDialog(&States::clear_dataset, &States::back_to_menu);
}; // namespace VibroScenario

void Common::InitVibroScenario::enterAction(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
}

void Common::InitVibroScenario::update(App *app) {
  p_grc = std::make_unique<VibroGrc>();
  int res = p_grc->init(g_HP_Ventilator);
  if (res < 0) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(&Common::States::select_scenario_menu);
  } else {
    assert(::g_HP_Ventilator.m_InputComponents == SIGNAL_COMPS_NUM);
    initGrcController(p_grc.get());
    GrcEvent_t grc_ev{eGrcEventId::LOAD_WGTS, app->p_storage.get()};
    xQueueSend(GrcControllerQueues.xRxQueue, &grc_ev, portMAX_DELAY);
    app->set_ctx(p_grc->getName());
    app->transition(&VibroScenario::States::menu);
  }
}
