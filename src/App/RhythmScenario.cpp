#include "App.hpp"
#include "RhythmGrc.hpp"
#include "preprocessing.hpp"

extern HP g_HP_Knock;
constexpr char TAG[] = "RhythmScenario";

namespace RhythmScenario {
struct States {
  static State &select_learn;
  static State &select_classify;
  static State &select_clear_dataset;

  static State &menu;
  static State &back_to_menu;
  static State &back_to_learn;
  static State &back_to_classify;

  static State &learn;
  static State &learn_listen;
  static State &learn_try_again;
  static State &try_memory_write;
  static State &save_dialog;
  static State &overwrite_sample;
  static State &overwrite_sample_dialog;
  static State &select_sample;
  static State &overwrite_dialog;
  static State &ask_to_switch;

  static State &classify;
  static State &classify_listen;
  static State &classify_try_again;
  static State &process_category;
  static State &process_no_match;
  static State &print_category;

  static State &clear_dataset_dialog;
  static State &clear_dataset;
};

static int s_category = -1;
static int s_cats_qty = -1;
static SignalData_t s_data;

struct Listen : Common::Listen {
  using Common::Listen::Listen;
  void updateDisplay(App *app) {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Press User to go");
    app->p_display->send();
  }
  void enterAction(App *app) override final {
    s_data.buffer = new float[s_data.num_rows * s_data.num_cols];
    Common::Listen::enterAction(app);
  }
  void handleEvent(App *app, eEvent ev) override final {
    switch (ev) {
    case eEvent::CMD_GO:
    case eEvent::BUTTON_CLICK: {
      delayMS(1000);
      xQueueSend(ListenerQueues.xRxQueue, &s_data, portMAX_DELAY);
      app->transition(forward_state_);
    } break;
    case eEvent::CMD_UP:
      if (back_state_ != nullptr) {
        if (s_data.buffer != nullptr) {
          delete[] s_data.buffer;
        }
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

struct TryAgain : State {
  TryAgain(State *back_state) : back_state_(back_state) {}
  void enterAction(App *app) override final {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Bad signal.");
    app->p_display->print_string("Try again.");
    app->p_display->send();
  }
  void update(App *app) override final {
    if (app->check_timeout(1000)) {
      app->transition(back_state_);
    }
  }

private:
  State *back_state_;
};

struct Learn : State {
  void enterAction(App *app) override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Learning is on...");
    app->p_display->send();
  }
  void update(App *app) override final {
    SignalData_t data;
    if (xQueueReceive(ListenerQueues.xTxQueue, &data, 0) == pdPASS) {
      LOGD(TAG, "Done: num_cols=%d", data.num_cols);
      MatrixDyn mat;
      makeMatrix(mat, data.num_rows, data.num_cols, data.buffer);
      // TODO: combine with preprocessing in RhythmGrc
      if (!preprocessRhythm(mat, data.sync_time_ms)) {
        delete[] data.buffer;
        app->transition(&States::learn_try_again);
      } else {
        GrcEvent_t load_ev{eGrcEventId::LOAD_SIGNAL, new SignalData_t(data)};
        xQueueSend(GrcControllerQueues.xRxQueue, &load_ev, portMAX_DELAY);
        GrcEvent_t train_ev{eGrcEventId::TRAIN,
                            new int(s_cats_qty < MAX_TRAINABLE_CATEGORIES
                                        ? s_cats_qty
                                        : s_cats_qty - 1)};
        xQueueSend(GrcControllerQueues.xRxQueue, &train_ev, portMAX_DELAY);
        app->transition(&States::save_dialog);
      }
    }
  }
};

struct SaveDialog : Common::ConfirmDialog {
  using Common::ConfirmDialog::ConfirmDialog;
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                 MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Save rhythm?");
    drawMenuItems(app);
    app->p_display->send();
  }
};

struct TryMemoryWrite : State {
  void update(App *app) override final {
    if (s_cats_qty + 1 > MAX_TRAINABLE_CATEGORIES) {
      app->transition(&States::overwrite_dialog);
    } else {
      GrcEvent_t save_ev{eGrcEventId::SAVE_WGTS, app->p_storage.get()};
      xQueueSend(GrcControllerQueues.xRxQueue, &save_ev, portMAX_DELAY);
      s_cats_qty++;
      app->transition(&RhythmScenario::States::ask_to_switch);
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
    app->transition(&RhythmScenario::States::ask_to_switch);
  }
};

struct Classify : State {
  void enterAction(App *app) override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Classification");
    app->p_display->print_string("is on...");
    app->p_display->send();
  }
  void update(App *app) override final {
    SignalData_t data;
    if (xQueueReceive(ListenerQueues.xTxQueue, &data, 0) == pdPASS) {
      LOGD(TAG, "Done: num_cols=%d", data.num_cols);
      MatrixDyn mat;
      makeMatrix(mat, data.num_rows, data.num_cols, data.buffer);
      // TODO: combine with preprocessing in RhythmGrc
      if (!preprocessRhythm(mat, data.sync_time_ms)) {
        delete[] data.buffer;
        app->transition(&States::classify_try_again);
      } else {
        GrcEvent_t load_ev{eGrcEventId::LOAD_SIGNAL, new SignalData_t(data)};
        xQueueSend(GrcControllerQueues.xRxQueue, &load_ev, portMAX_DELAY);
        GrcEvent_t infer_ev{eGrcEventId::INFER, nullptr};
        xQueueSend(GrcControllerQueues.xRxQueue, &infer_ev, portMAX_DELAY);
        app->transition(&States::process_category);
      }
    }
  }
};

struct ProcessCategory : State {
  void update(App *app) override final {
    s_category = Common::process_grc_replies(eGrcReplyId::CATEGORY);
    if (s_category < 0) {
      app->transition(&RhythmScenario::States::process_no_match);
    } else {
      app->transition(&RhythmScenario::States::print_category);
    }
  }
};

struct SelectLearn : State {
  static constexpr const char *name = "Learn";

  void exitAction(App *app) override final { app->set_ctx(name); }
  void update(App *app) override final {
    app->transition(&States::learn_listen);
  }
};

struct SelectClassify : State {
  static constexpr const char *name = "Classify";

  void exitAction(App *app) override final { app->set_ctx(name); }
  void update(App *app) override final {
    app->transition(&States::classify_listen);
  }
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

struct AskToSwitch : Common::Menu {
  using Common::Menu::Menu;
  void enterAction(App *app) override final {
    Common::Menu::enterAction(app);
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
  }
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("R%d is saved", s_cats_qty);
    drawMenuItems(app);
    app->p_display->send();
  }
};

struct ProcessNoMatch : Common::Menu {
  using Common::Menu::Menu;
  void enterAction(App *app) override final {
    Common::Menu::enterAction(app);
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
  }
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("No match");
    drawMenuItems(app);
    app->p_display->send();
  }
};

struct PrintCategory : Common::Menu {
  using Common::Menu::Menu;
  void enterAction(App *app) override final {
    Common::Menu::enterAction(app);
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
  }
  void drawMenu(App *app) const override final {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Rhythm %d", s_category + 1);
    drawMenuItems(app);
    app->p_display->send();
  }
};

State &States::select_learn = *new SelectLearn;
State &States::select_classify = *new SelectClassify;
State &States::select_clear_dataset = *new SelectClearDataset;
State &States::menu = *new MainMenu(
    {
        {SelectLearn::name, &States::select_learn},
        {SelectClassify::name, &States::select_classify},
        {SelectClearDataset::name, &States::select_clear_dataset},
    },
    nullptr);
State &States::back_to_menu = *new Common::BackTo(&States::menu);
State &States::back_to_learn = *new Common::BackTo(&States::select_learn);
State &States::back_to_classify = *new Common::BackTo(&States::select_classify);

State &States::learn = *new Learn;
State &States::learn_listen =
    *new Listen(&States::learn, &States::back_to_menu);
State &States::learn_try_again = *new TryAgain(&States::learn_listen);
State &States::try_memory_write = *new TryMemoryWrite;
State &States::save_dialog =
    *new SaveDialog(&States::try_memory_write, &States::learn_listen);
State &States::overwrite_sample = *new OverwriteSample;
State &States::overwrite_dialog =
    *new OverwriteDialog(&States::overwrite_sample, &States::learn_listen);
State &States::ask_to_switch = *new AskToSwitch(
    {
        {SelectLearn::name, &States::back_to_learn},
        {SelectClassify::name, &States::back_to_classify},
    },
    &States::back_to_learn);

State &States::classify = *new Classify;
State &States::classify_listen =
    *new Listen(&States::classify, &States::back_to_menu);
State &States::classify_try_again = *new TryAgain(&States::classify_listen);
State &States::process_category = *new ProcessCategory;
State &States::process_no_match = *new ProcessNoMatch(
    {
        {"Try again", &States::back_to_classify},
        {"Go to Learn", &States::back_to_learn},
    },
    &States::back_to_classify);
State &States::print_category = *new PrintCategory(
    {
        {"Try again", &States::back_to_classify},
        {"Go to Learn", &States::back_to_learn},
    },
    &States::back_to_classify);

State &States::clear_dataset = *new ClearDataset;
State &States::clear_dataset_dialog =
    *new ClearDatasetDialog(&States::clear_dataset, &States::back_to_menu);
} // namespace RhythmScenario

void Common::InitRhythmScenario::enterAction(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
}

void Common::InitRhythmScenario::update(App *app) {
  p_grc = std::make_unique<RhythmGrc>();
  int res = p_grc->init(g_HP_Knock);
  if (res < 0) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(&Common::States::select_scenario_menu);
  } else {
    RhythmScenario::s_data.num_rows = SIGNAL_COMPS_NUM;
    RhythmScenario::s_data.num_cols = SIGNAL_SAMPLES_NUM;
    RhythmScenario::s_data.sync_time_ms = SIGNAL_PERIOD_MS;
    initGrcController(p_grc.get());
    GrcEvent_t grc_ev{eGrcEventId::LOAD_WGTS, app->p_storage.get()};
    xQueueSend(GrcControllerQueues.xRxQueue, &grc_ev, portMAX_DELAY);
    app->set_ctx(p_grc->getName());
    app->transition(&RhythmScenario::States::menu);
  }
}
