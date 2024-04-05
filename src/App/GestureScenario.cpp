#include "App.hpp"
#include "GestureGrc.hpp"

#include "kws_preprocessor.h"

static constexpr char TAG[] = "GestureScenario";

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
static void try_again_msg(App *app);

static void prep_state(App *app) {
  app->p_display->print_header("%s %d/%d", app->get_ctx(), s_cats_qty,
                               MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("Prepare");
  app->p_display->send();
  xEventGroupSetBits(xStatusEventGroup, STATUS_PREP_STATE_MSK);

  const unsigned FRAG_LEN = 300;
  const float MARG = 0.015F;
  float *frag_buffer = new float[FRAG_LEN * SIGNAL_COMPS_NUM];
  SignalData_t data = {frag_buffer, SIGNAL_COMPS_NUM, FRAG_LEN,
                       SIGNAL_PERIOD_MS};
  for (;;) {
    xQueueSend(ListenerQueues.xRxQueue, &data, portMAX_DELAY);
    if (xQueueReceive(ListenerQueues.xTxQueue, &data, portMAX_DELAY) ==
        pdPASS) {
      Matrix signal = convert(data, false, false);
      Matrix avg_mat, std_mat;
      signal.calcStat(avg_mat, StatMMAS<Matrix::ElmType>::Avg, false, true);
      Matrix(signal, Matrix::Rect(3, 0), false)
          .calcStat(std_mat, StatMMAS<Matrix::ElmType>::StDev, false, true);

      Matrix mrg_mat;
      std_mat.calcStat(mrg_mat, StatMMAS<Matrix::ElmType>::Max, false, false);

      if ((float)mrg_mat[0] < MARG)
        break;
    }
  }
  delete[] frag_buffer;

  xEventGroupClearBits(xStatusEventGroup, STATUS_PREP_STATE_MSK);
}

namespace GestureScenario {
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
      VoiceMsgPlay(14 + s_category);
      VoiceMsgPlay(20);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      Common::Menu::enterAction(app);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Gesture %d", s_category + 1);
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
      app->p_display->print_string("Save gesture?");
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

  struct ReadyToListen : State {
    ReadyToListen(action_t forward_action, action_t back_action, VoiceMsgId voice_msg)
      : forward_action_(forward_action), back_action_(back_action), voice_msg_(voice_msg) {}
    State* clone() override final {
      return new ReadyToListen(*this);
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
      case eEvent::CMD_YES: {
        VoiceMsgPlay(4);
        VoiceMsgWaitStop(portMAX_DELAY);
        prep_state(app);
        forward_action_(app);
      } break;
      case eEvent::BUTTON_TO_DOWN: {
        VoiceMsgStop();
        kws_req_cancel();
        prep_state(app);
        forward_action_(app);
      } break;
      case eEvent::CMD_UP:
        if (back_action_ != nullptr) {
          back_action_(app);
        } else {
          updateDisplay(app);
        }
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

  private:
    action_t forward_action_;
    action_t back_action_;
    VoiceMsgId voice_msg_;
  };
   
  struct Learn : State {
    State* clone() override final {
      return new Learn(*this);
    }
    void enterAction(App *app) override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Learning...");
      app->p_display->send();
      SignalData_t data = {
        .buffer = new float[SIGNAL_COMPS_NUM * SIGNAL_SAMPLES_NUM],
        .num_rows = SIGNAL_COMPS_NUM,
        .num_cols = SIGNAL_SAMPLES_NUM,
        .sync_time_ms = SIGNAL_PERIOD_MS,
      };
      if (xQueueSend(ListenerQueues.xRxQueue, &data, 0) == pdFAIL) {
        LOGE(TAG, "Error sending to data listener");
      }
    }
    void handleEvent(App *app, eEvent ev) override final {
      switch (ev) {
      case eEvent::BUTTON_TO_UP:
        xEventGroupSetBits(xListenerEventGroup, LISTENER_STOP_MSK);
        break;
      default:
        break;
      }
    }
    void update(App *app) override final {
      SignalData_t data;
      if (xQueueReceive(ListenerQueues.xTxQueue, &data, 0) == pdPASS) {
        LOGD(TAG, "Done: num_cols=%d", data.num_cols);
        Matrix signal = convert(data, false, true);
        if (data.buffer && (data.num_cols * data.sync_time_ms < 500 ||
                            getSignalBeg(signal, 20) < 0)) {
          try_again_msg(app);
          select_learn(app);
          return;
        }
        int ret = grc_helper_train(s_grc, signal, s_cats_qty < MAX_TRAINABLE_CATEGORIES
                                                  ? s_cats_qty
                                                  : s_cats_qty - 1);
        if (ret < 0) {
          LOGE(TAG, "train error: %d", ret);
          try_again_msg(app);
          select_learn(app);
          return;
        }
        app->transition(new GestureScenario::SaveDialog(&try_memory_write, &select_learn, 6));
      }
    }
  };

  struct Classify : State {
    State* clone() override final {
      return new Classify(*this);
    }
    void enterAction(App *app) override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Classification");
      app->p_display->print_string("is on...");
      app->p_display->send();
      SignalData_t data = {
        .buffer = new float[SIGNAL_COMPS_NUM * SIGNAL_SAMPLES_NUM],
        .num_rows = SIGNAL_COMPS_NUM,
        .num_cols = SIGNAL_SAMPLES_NUM,
        .sync_time_ms = SIGNAL_PERIOD_MS,
      };
      if (xQueueSend(ListenerQueues.xRxQueue, &data, 0) == pdFAIL) {
        LOGE(TAG, "Error sending to data listener");
      }
    }
    void handleEvent(App *app, eEvent ev) override final {
      switch (ev) {
      case eEvent::BUTTON_TO_UP:
        xEventGroupSetBits(xListenerEventGroup, LISTENER_STOP_MSK);
        break;
      default:
        break;
      }
    }
    void update(App *app) override final {
      SignalData_t data;
      if (xQueueReceive(ListenerQueues.xTxQueue, &data, 0) == pdPASS) {
        LOGD(TAG, "Done: num_cols=%d", data.num_cols);
        Matrix signal = convert(data, false, true);
        if (data.buffer && (data.num_cols * data.sync_time_ms < 500 ||
                            getSignalBeg(signal, 20) < 0)) {
          try_again_msg(app);
          select_classify(app);
          return;
        }
        s_category = grc_helper_infer(s_grc, signal);
        if (s_category < 0) {
          LOGI(TAG, "No match");
          app->transition(new GestureScenario::ProcessNoMatch(
            {
                {"Try again", &select_classify},
                {"Go to Learn", &select_learn},
            },
            {select_classify_str, &select_classify})
          );
        } else {
          LOGI(TAG, "Gesture %d", s_category + 1);
          app->transition(new GestureScenario::PrintCategory(
            {
                {"Try again", &select_classify},
                {"Go to Learn", &select_learn},
            },
            {select_classify_str, &select_classify})
          );
        }
      }
    }
  };

  struct AskToSwitch : Common::Menu {
    using Common::Menu::Menu;
    State* clone() override final {
      return new AskToSwitch(*this);
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(8 + s_cats_qty - 1);
      VoiceMsgPlay(7);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      Common::Menu::enterAction(app);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("G%d is saved", s_cats_qty);
      drawMenuItems(app);
      app->p_display->send();
    }
  };
} // namespace GestureScenario

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

static void select_learn(App *app) {
  app->pop_ctx();
  app->set_ctx(select_learn_str);
  app->transition(new GestureScenario::ReadyToListen(
    [] (App *app) {
      app->transition(new GestureScenario::Learn);
    },
    [] (App *app) {
      app->transition(getMenu());
    }, 3)
    );
}

static void select_classify(App *app) {
  app->pop_ctx();
  app->set_ctx(select_classify_str);
  app->transition(new GestureScenario::ReadyToListen(
    [] (App *app) {
      app->transition(new GestureScenario::Classify);
    },
    [] (App *app) {
      app->transition(getMenu());
    }, 3)
    );
}

static void save_samples(App *app) {
  grc_helper_save_wgts(s_grc, app->p_storage.get());
  app->transition(getAskToSwitchMenu());
}

static void try_memory_write(App *app) {
  if (s_cats_qty + 1 > MAX_TRAINABLE_CATEGORIES) {
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(new GestureScenario::OverwriteDialog(&save_samples, &select_learn, 13));
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
  return new GestureScenario::MainMenu(
    {
        {select_learn_str, &select_learn},
        {select_classify_str, &select_classify},
        {select_clr_data_str, [](App *app) {
          app->set_ctx(select_clr_data_str);
          app->transition(new GestureScenario::ClearDatasetDialog(&clear_dataset, &back_to_menu, 21));
          }
        },
    },
    {"Back", nullptr}, 2);
}

static State* getAskToSwitchMenu() {
  return new GestureScenario::AskToSwitch(
    {
        {select_learn_str, &select_learn},
        {select_classify_str, &select_classify},
    },
    {select_learn_str, &select_learn});
}

extern const int gesture_len;
extern const uint8_t gesture_bytes[];

void Common::initGestureScenario(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
  s_grc = new GestureGrc;
  int res = s_grc->init(GestureGrc::hp, gesture_bytes, gesture_len);
  if (res < 0) {
    LOGE(TAG, "Gesture init error");
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(Common::getMainMenu());
  } else {
    grc_helper_load_wgts(s_grc, app->p_storage.get());
    s_cats_qty = s_grc->getQty();
    app->set_ctx(s_grc->getName());
    app->transition(getMenu());
  }
}
