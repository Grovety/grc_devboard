#include "App.hpp"
#include "State.hpp"
#include "VibroGrc.hpp"
#include "kws_preprocessor.h"
#include "i2s_rx_slot.h"

#include "string.h"

static constexpr char TAG[] = "VibroScenario";

static int s_category = -1;
static int s_cats_qty = -1;
static BaseGrc *s_grc = nullptr;
static float *s_buffer_ptr = nullptr;
static SemaphoreHandle_t xADSema;

static State* getMenu();
static void transition_to_adloop(App *app);

namespace VibroScenario {
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
    void enterAction(App *app) override final {
      Common::ConfirmDialog::enterAction(app);
      xTimerChangePeriod(xTimer, pdMS_TO_TICKS(8000), 0);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Anomaly");
      app->p_display->print_string("Learn pattern?");
      drawMenuItems(app);
      app->p_display->send();
    }
    void handleEvent(App *app, eEvent ev) override final {
      switch (ev) {
      case eEvent::BUTTON_HOLD:
      case eEvent::CMD_YES:
      case eEvent::CMD_NO:
        xTimerStop(xTimer, 0);
        break;
      default:
        break;
      }
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
      case eEvent::TIMEOUT:
        kws_req_cancel();
        items_[1].action(app);
        break;
      case eEvent::CMD_ONE:
      case eEvent::CMD_TWO:
      case eEvent::CMD_THREE:
      case eEvent::CMD_SHEILA:
      case eEvent::CMD_UNKNOWN:
        drawMenu(app);
        if (uxQueueMessagesWaiting(xKWSRequestQueue) == 0) {
          kws_req_word(1);
        }
        break;
      default:
        drawMenu(app);
        break;
      }
    }
  };

  struct OverwriteDialog : Common::ConfirmDialog {
    using Common::ConfirmDialog::ConfirmDialog;
    State* clone() override final {
      return new OverwriteDialog(*this);
    }
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

  struct ReadyToStart : State {
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
      VoiceMsgPlay(3);
      updateDisplay(app);
      kws_req_word(1);
    }
    void exitAction(App *app) override final {
      xEventGroupSetBits(xStatusEventGroup, STATUS_SYSTEM_BUSY_MSK);
    }
    void handleEvent(App *app, eEvent ev) override final {
      switch (ev) {
      case eEvent::CMD_YES:
        VoiceMsgStop();
        transition_to_adloop(app);
        break;
      case eEvent::BUTTON_CLICK:
        kws_req_cancel();
        vTaskDelay(pdMS_TO_TICKS(500));
        transition_to_adloop(app);
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
  };

  struct ADLoop : State {
    State* clone() override final {
      return new ADLoop(*this);
    }
    void enterAction(App *app) override final {
      LOGD(TAG, "ADLoop state");
      xSemaphoreGive(xADSema);
    }
  };
} // namespace VibroScenario

static void transition_to_adloop(App *app) {
  app->transition(new VibroScenario::ADLoop);
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

static void save_samples(App *app) {
  VoiceMsgPlay(43 + s_cats_qty - 1);
  xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
  app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
  app->p_display->print_string("Pattern %d saved", s_cats_qty);
  app->p_display->send();
  grc_helper_save_wgts(s_grc, app->p_storage.get());
  VoiceMsgWaitStop(portMAX_DELAY);
  transition_to_adloop(app);
}

static State* getMenu() {
  static constexpr const char *select_adloop_str = "AD";
  static constexpr const char *select_clr_data_str = "Clr data";
  return new VibroScenario::MainMenu(
    {
        {select_adloop_str, [](App *app) {
          app->set_ctx(select_adloop_str);
          app->transition(new VibroScenario::ReadyToStart);
          }
        },
        {select_clr_data_str, [](App *app) {
          app->set_ctx(select_clr_data_str);
          app->transition(new VibroScenario::ClearDatasetDialog(&clear_dataset, &back_to_menu, 21));
          }
        },
    },
    {"Back", nullptr}, 2);
}

static void listen_signal(Matrix &signal) {
  SignalData_t data = SignalData_t{
    .buffer = s_buffer_ptr,
    .num_rows = SIGNAL_COMPS_NUM,
    .num_cols = SIGNAL_SAMPLES_NUM,
    .sync_time_ms = SIGNAL_PERIOD_MS,
  };
  xSemaphoreTake(xMicSema, portMAX_DELAY);
  xQueueSend(ListenerQueues.xRxQueue, &data, portMAX_DELAY);
  xQueueReceive(ListenerQueues.xTxQueue, &data, portMAX_DELAY);
  LOGD(TAG, "Done: num_cols=%d", data.num_cols);
  xSemaphoreGive(xMicSema);
  signal = convert(data, false, false);
}

static void train(App *app) {
  SignalData_t data = SignalData_t{
    .buffer = s_buffer_ptr,
    .num_rows = SIGNAL_COMPS_NUM,
    .num_cols = SIGNAL_SAMPLES_NUM,
    .sync_time_ms = SIGNAL_PERIOD_MS,
  };
  Matrix signal = convert(data, false, false);
  int ret = grc_helper_train(s_grc, signal, s_cats_qty < MAX_TRAINABLE_CATEGORIES
                                            ? s_cats_qty
                                            : s_cats_qty - 1);
  if (ret < 0) {
    LOGE(TAG, "train error: %d", ret);
  }
  if (s_cats_qty + 1 > MAX_TRAINABLE_CATEGORIES) {
    app->transition(new VibroScenario::OverwriteDialog(&save_samples, &transition_to_adloop, 13));
  } else {
    s_cats_qty++;
    save_samples(app);
  }
}

static void anomaly_detection_task(void *pv) {
  App *app = static_cast<App*>(pv);

  xSemaphoreTake(xADSema, portMAX_DELAY);
  for (;;) {
    app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
    app->p_display->print_string("Detecting...");
    app->p_display->send();
    VoiceMsgPlay(35);
    Matrix signal;
    listen_signal(signal);
    const int category = grc_helper_infer(s_grc, signal);
    vTaskDelay(pdMS_TO_TICKS(500));

    if (category <= -1 || category > (MAX_TRAINABLE_CATEGORIES - 1)) {
      s_category = -1;
      LOGI(TAG, "Anomaly!");
      VoiceMsgPlay(36);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
      if (s_cats_qty < MAX_TRAINABLE_CATEGORIES) {
        app->transition(new VibroScenario::SaveDialog(&train, &transition_to_adloop, 37));
        xSemaphoreTake(xADSema, portMAX_DELAY);
      } else {
        app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
        app->p_display->print_string("Anomaly");
        app->p_display->send();
        VoiceMsgWaitStop(portMAX_DELAY);
      }
    } else if (category != s_category) {
      s_category = category;
      LOGI(TAG, "Mode %d", category + 1);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Pattern %d", s_category + 1);
      app->p_display->send();
      VoiceMsgPlay(38 + s_category);
      VoiceMsgWaitStop(portMAX_DELAY);
    } else {
      LOGI(TAG, "Mode %d", category + 1);
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      app->p_display->print_header("%d/%d", s_cats_qty, MAX_TRAINABLE_CATEGORIES);
      app->p_display->print_string("Pattern %d", s_category + 1);
      app->p_display->send();
      VoiceMsgPlay(38 + s_category);
      VoiceMsgWaitStop(portMAX_DELAY);
    }
    vTaskDelay(1);
  }
}

extern const int vibro_len;
extern const uint8_t vibro_bytes[];

void Common::initVibroScenario(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
  s_grc = new VibroGrc;
  s_buffer_ptr = new float[SIGNAL_COMPS_NUM * SIGNAL_SAMPLES_NUM];
  int res = s_grc->init(VibroGrc::hp, vibro_bytes, vibro_len);
  if (res < 0 || !s_grc || !s_buffer_ptr) {
    LOGE(TAG, "Vibro init error");
    xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
    app->transition(Common::getMainMenu());
  } else {
    xADSema = xSemaphoreCreateBinary();
    if (!xADSema) {
      LOGE(TAG, "Error creating xADSema");
    }
    auto xReturned = 
      xTaskCreate(anomaly_detection_task, "anomaly_detection_task", configMINIMAL_STACK_SIZE + 1024 * 16, app, 2, NULL);
    if (xReturned != pdPASS) {
      LOGE(TAG, "Error creating anomaly_detection_task");
    }
    grc_helper_load_wgts(s_grc, app->p_storage.get());
    s_cats_qty = s_grc->getQty();
    app->set_ctx(s_grc->getName());
    app->transition(getMenu());
  }
}
