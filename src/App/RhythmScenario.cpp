#include "App.hpp"
#include "RhythmGrc.hpp"

#include "kws_preprocessor.h"

static constexpr char TAG[] = "RhythmScenario";

static constexpr const char *select_learn_str = "Learn";
static constexpr const char *select_classify_str = "Classify";
static constexpr const char *select_clr_data_str = "Clr data";

static int s_category = -1;
static int s_cats_qty = -1;
static BaseGrc *s_grc = nullptr;

static State* getMenu();
static State* getAskToSwitchMenu();
static void select_learn(App *app);
static void select_classify(App *app);
static void try_memory_write(App *app);

static void printSig(const Matrix &buf, int cat = -1) {
  if (cat == -1) {
    printf("Rythm:    I: ");
  } else {
    printf("Rythm: T[%d]: ", cat + 1);
  }
  unsigned qty = buf.getCols() / 16 - 2;
  for (unsigned i = 0; i < qty; i += 2)
    printf("%d ", (int)(100 * (float)buf(0, i)));
  printf("\n");
}

namespace RhythmScenario {
  struct MainMenu : Common::Menu {
    using Common::Menu::Menu;
    State* clone() override final {
      return new MainMenu(*this);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                  MAX_TRAINABLE_CATEGORIES);
      drawMenuItems(app);
      app->p_display->send();
    }
  };

  struct ProcessNoMatch : Common::Menu {
    using Common::Menu::Menu;
    State* clone() override final {
      return new ProcessNoMatch(*this);
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(19);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
      Common::Menu::enterAction(app);
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
    State* clone() override final {
      return new PrintCategory(*this);
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(30 + s_category);
      VoiceMsgPlay(20);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      Common::Menu::enterAction(app);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Rhythm %d", s_category + 1);
      drawMenuItems(app);
      app->p_display->send();
    }
  };

  struct ClearDatasetDialog : Common::ConfirmDialog {
    using Common::ConfirmDialog::ConfirmDialog;
    State* clone() override final {
      return new ClearDatasetDialog(*this);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                  MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Are you sure?");
      drawMenuItems(app);
      app->p_display->send();
    }
  };

  struct SaveDialog : Common::ConfirmDialog {
    using Common::ConfirmDialog::ConfirmDialog;
    State* clone() override final {
      return new SaveDialog(*this);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                  MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Save rhythm?");
      drawMenuItems(app);
      app->p_display->send();
    }
  };

  struct OverwriteDialog : Common::ConfirmDialog {
    using Common::ConfirmDialog::ConfirmDialog;
    State* clone() override final {
      return new OverwriteDialog(*this);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Memory is full.");
      app->p_display->print_string("Overwrite last?");
      drawMenuItems(app);
      app->p_display->send();
    }
  };
   
  struct ReadyToStart : State {
    ReadyToStart(action_t action, VoiceMsgId voice_msg)
      : action_(action), voice_msg_(voice_msg) {}
    State* clone() override final {
      return new ReadyToStart(*this);
    }
    void updateDisplay(App *app) {
      app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                  MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Are you ready?");
      app->p_display->send();
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(voice_msg_);
      updateDisplay(app);
      kws_req_word(1);
    }
    void handleEvent(App *app, eEvent ev) override final {
      switch (ev) {
      case eEvent::BUTTON_CLICK:
        VoiceMsgStop();
        kws_req_cancel();
        vTaskDelay(pdMS_TO_TICKS(500));
        action_(app);
        break;
      case eEvent::CMD_YES:
        VoiceMsgPlay(4);
        action_(app);
        break;
      case eEvent::CMD_UP:
        app->transition(getMenu());
        break;
      case eEvent::CMD_UNKNOWN:
      case eEvent::CMD_NO:
      case eEvent::CMD_ONE:
      case eEvent::CMD_TWO:
      case eEvent::CMD_THREE:
      case eEvent::CMD_FOUR:
      case eEvent::CMD_SHEILA: {
        updateDisplay(app);
        if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
          kws_req_word(1);
        }
      } break;
      default:
        break;
      }
    }

  protected:
    action_t action_;
    VoiceMsgId voice_msg_;
  };

  struct AskToSwitch : Common::Menu {
    using Common::Menu::Menu;
    State* clone() override final {
      return new AskToSwitch(*this);
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(25 + s_cats_qty - 1);
      VoiceMsgPlay(7);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      Common::Menu::enterAction(app);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("R%d is saved", s_cats_qty);
      drawMenuItems(app);
      app->p_display->send();
    }
  };
} // namespace RhythmScenario

static void listen_signal(Matrix &signal) {
  SignalData_t data = SignalData_t{
    .buffer = new float[SIGNAL_SAMPLES_NUM * SIGNAL_COMPS_NUM],
    .num_rows = SIGNAL_COMPS_NUM,
    .num_cols = SIGNAL_SAMPLES_NUM,
    .sync_time_ms = SIGNAL_PERIOD_MS,
  };
  xQueueSend(ListenerQueues.xRxQueue, &data, portMAX_DELAY);
  xQueueReceive(ListenerQueues.xTxQueue, &data, portMAX_DELAY);
  LOGD(TAG, "Done: num_cols=%d", data.num_cols);
  signal = convert(data, false, true);
}

static void try_again_msg(App *app) {
  VoiceMsgPlay(23);
  xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
  app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("Bad signal.");
  app->p_display->print_string("Try again.");
  app->p_display->send();
  VoiceMsgWaitStop(portMAX_DELAY);
}

static void learn(App *app) {
  app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("Learning...");
  app->p_display->send();

  Matrix signal;
  listen_signal(signal);
  if (!preprocessRhythm(signal, SIGNAL_PERIOD_MS)) {
    try_again_msg(app);
    select_learn(app);
    return;
  }

#ifdef PRINT_RHYTHM_SIG
  printSig(signal, s_cats_qty);
#endif
  int ret = grc_helper_train(s_grc, signal, s_cats_qty < MAX_TRAINABLE_CATEGORIES
                                            ? s_cats_qty
                                            : s_cats_qty - 1);
  if (ret < 0) {
    LOGE(TAG, "train error: %d", ret);
  }
  app->transition(new RhythmScenario::SaveDialog(&try_memory_write, &select_learn, 24));
}

static void classify(App *app) {
  app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("Classification");
  app->p_display->print_string("is on...");
  app->p_display->send();

  Matrix signal;
  listen_signal(signal);
  if (!preprocessRhythm(signal, SIGNAL_PERIOD_MS)) {
    try_again_msg(app);
    select_classify(app);
    return;
  }

#ifdef PRINT_RHYTHM_SIG
  printSig(signal);
#endif
  s_category = grc_helper_infer(s_grc, signal);
  if (s_category < 0) {
    LOGI(TAG, "No match");
    app->transition(new RhythmScenario::ProcessNoMatch(
      {
          {"Try again", &select_classify},
          {"Go to Learn", &select_learn},
      },
      {select_classify_str, &select_classify})
    );
  } else {
    LOGI(TAG, "Rhythm %d", s_category + 1);
    app->transition(new RhythmScenario::PrintCategory(
      {
          {"Try again", &select_classify},
          {"Go to Learn", &select_learn},
      },
      {select_classify_str, &select_classify})
    );
  }
}

static void select_learn(App *app) {
  app->pop_ctx();
  app->set_ctx(select_learn_str);
  app->transition(new RhythmScenario::ReadyToStart(&learn, 3));
}

static void select_classify(App *app) {
  app->pop_ctx();
  app->set_ctx(select_classify_str);
  app->transition(new RhythmScenario::ReadyToStart(&classify, 3));
}

static void save_samples(App *app) {
  grc_helper_save_wgts(s_grc, app->p_storage.get());
  app->transition(getAskToSwitchMenu());
}

static void try_memory_write(App *app) {
  if (s_cats_qty + 1 > MAX_TRAINABLE_CATEGORIES) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(new RhythmScenario::OverwriteDialog(&save_samples, &select_learn, 13));
  } else {
    s_cats_qty++;
    save_samples(app);
  }
}

static void back_to_menu(App *app) {
  xEventGroupClearBits(xStatusEventGroup, STATUS_SYSTEM_BUSY_MSK);
  app->pop_ctx();
  app->transition(getMenu());
}

static void clear_dataset(App *app) {
  app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                                MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("The dataset");
  app->p_display->print_string("is deleted");
  app->p_display->send();
  VoiceMsgPlay(22);
  grc_helper_clear_wgts(s_grc, app->p_storage.get());
  s_cats_qty = s_grc->getQty();
  xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
  VoiceMsgWaitStop(portMAX_DELAY);
  back_to_menu(app);
}

static State* getMenu() {
  return new RhythmScenario::MainMenu(
    {
        {select_learn_str, &select_learn},
        {select_classify_str, &select_classify},
        {select_clr_data_str, [](App *app) {
          app->set_ctx(select_clr_data_str);
          app->transition(new RhythmScenario::ClearDatasetDialog(&clear_dataset, &back_to_menu, 21));
          }
        },
    },
    {"Back", nullptr}, 2);
}

static State* getAskToSwitchMenu() {
  return new RhythmScenario::AskToSwitch(
    {
        {select_learn_str, &select_learn},
        {select_classify_str, &select_classify},
    },
    {select_learn_str, &select_learn});
}

extern const int knock_len;
extern const uint8_t knock_bytes[];

void Common::initRhythmScenario(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
  s_grc = new RhythmGrc;
  int res = s_grc->init(RhythmGrc::hp, knock_bytes, knock_len);
  if (res < 0) {
    LOGE(TAG, "Rhythm init error");
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(Common::getMainMenu());
  } else {
    grc_helper_load_wgts(s_grc, app->p_storage.get());
    s_cats_qty = s_grc->getQty();
    app->set_ctx(s_grc->getName());
    app->transition(getMenu());
  }
}
