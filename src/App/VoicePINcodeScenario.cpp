#include "App.hpp"
#include "Status.hpp"
#include "VoiceMsgPlayer.hpp"
#include "kws_preprocessor.h"

#include "string.h"

#define PIN_LEN 4

static constexpr char TAG[] = "VoicePINcode";

static constexpr const char *select_learn_str = "Learn";
static constexpr const char *select_classify_str = "Classify";

extern const unsigned char *kws_num_model_ptr;
extern const char *kws_num_labels[];
extern unsigned int kws_num_labels_num;

extern const unsigned char *kws_cmd_model_ptr;
extern const char *kws_cmd_labels[];
extern unsigned int kws_cmd_labels_num;

static char placeholder_str[] = "_ _ _ _";
static char pin_str[sizeof(placeholder_str)] = {0};
static int s_saved_pin[PIN_LEN] = {0};

static State* getSaveDialog();

static int get_int_from_kws_result(const char *result) {
  int ret = -1;
  if (strcmp(result, "zero") == 0) {
    ret = 0;
  } else if (strcmp(result, "one") == 0) {
    ret = 1;
  } else if (strcmp(result, "two") == 0) {
    ret = 2;
  } else if (strcmp(result, "three") == 0) {
    ret = 3;
  } else if (strcmp(result, "four") == 0) {
    ret = 4;
  } else if (strcmp(result, "five") == 0) {
    ret = 5;
  } else if (strcmp(result, "six") == 0) {
    ret = 6;
  } else if (strcmp(result, "seven") == 0) {
    ret = 7;
  } else if (strcmp(result, "eight") == 0) {
    ret = 8;
  } else if (strcmp(result, "nine") == 0) {
    ret = 9;
  }
  return ret;
}

static void record_numbers(App *app) {
  app->set_ctx(select_learn_str);
  kws_model_release();
  kws_model_init(kws_num_model_ptr, kws_num_labels, kws_num_labels_num);

  char result[32];

  xSemaphoreTake(xKWSSema, portMAX_DELAY);
  while (1) {
    strcpy(pin_str, placeholder_str);
    app->p_display->print_header("%s", app->get_ctx());
    app->p_display->print_string("%s", pin_str);
    app->p_display->send();
    LOGI(TAG, "Say %d numbers", PIN_LEN);
    kws_req_word(PIN_LEN);
    for (size_t i = 0; i < PIN_LEN; i++) {
      xSemaphoreTake(xKWSWordSema, portMAX_DELAY);
      pin_str[i * 2] = '*';
      app->p_display->print_header("%s", app->get_ctx());
      app->p_display->print_string("%s", pin_str);
      app->p_display->send();
    }

    bool stop = true;
    for (size_t i = 0; i < PIN_LEN; i++) {
      memset(result, 0, sizeof(result));
      int category;
      xQueueReceive(xKWSResultQueue, &category, portMAX_DELAY);
      kws_get_category(category, result, sizeof(result));
      const int pin_val = get_int_from_kws_result(result);
      s_saved_pin[i] = pin_val;
      if (pin_val < 0 || pin_val > 9) {
        LOGI(TAG, "bad pin_value[%d]=%d", i, pin_val);
        pin_str[i * 2] = '-';
        stop = false;
      } else {
        pin_str[i * 2] = '0' + pin_val;
      }
      app->p_display->print_header("%s", app->get_ctx());
      app->p_display->print_string("%s", pin_str);
      app->p_display->send();
    }
    if (stop) {
      break;
    } else {
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
      VoiceMsgPlay(60);
      VoiceMsgWaitStop(portMAX_DELAY);
    }
  }
  LOGI(TAG, "recored pin: %d, %d, %d, %d", s_saved_pin[0], s_saved_pin[1], s_saved_pin[2], s_saved_pin[3]);
  xSemaphoreGive(xKWSSema);

  kws_model_release();
  kws_model_init(kws_cmd_model_ptr, kws_cmd_labels, kws_cmd_labels_num);
  app->pop_ctx();
  app->transition(getSaveDialog());
}

static int check_pin(App *app) {
  kws_model_release();
  kws_model_init(kws_num_model_ptr, kws_num_labels, kws_num_labels_num);

  char result[32];
  int read_pin[PIN_LEN] = {0};
  char pin_str[sizeof(placeholder_str)] = {0};
  strcpy(pin_str, placeholder_str);
  app->p_display->print_status("");
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->print_string("%s", pin_str);
  app->p_display->send();

  xSemaphoreTake(xKWSSema, portMAX_DELAY);
  LOGI(TAG, "Say %d numbers", PIN_LEN);
  kws_req_word(PIN_LEN);
  for (size_t i = 0; i < PIN_LEN; i++) {
    xSemaphoreTake(xKWSWordSema, portMAX_DELAY);
    pin_str[i * 2] = '*';
    app->p_display->print_header("%s", app->get_ctx());
    app->p_display->print_string("%s", pin_str);
    app->p_display->send();
  }
  for (size_t i = 0; i < PIN_LEN; i++) {
    memset(result, 0, sizeof(result));
    int category;
    xQueueReceive(xKWSResultQueue, &category, portMAX_DELAY);
    kws_get_category(category, result, sizeof(result));
    read_pin[i] = get_int_from_kws_result(result);
  }
  xSemaphoreGive(xKWSSema);

  int ret = 0;
  for (size_t i = 0; i < PIN_LEN; ++i) {
    if (read_pin[i] != s_saved_pin[i]) {
      ret = -1;
    }
  }

  kws_model_release();
  kws_model_init(kws_cmd_model_ptr, kws_cmd_labels, kws_cmd_labels_num);
  return ret;
}

namespace VoicePINcode {
  struct SaveDialog : Common::ConfirmDialog {
    using Common::ConfirmDialog::ConfirmDialog;
    State* clone() override final {
      return new SaveDialog(*this);
    }
    void enterAction(App *app) override final {
      VoiceMsgPlay(48);
      for (size_t i = 0; i < PIN_LEN; ++i) {
        VoiceMsgPlay(49 + s_saved_pin[i]);
      }
      Common::Menu::enterAction(app);
    }
    void drawMenu(App *app) const override final {
      app->p_display->print_header("%s", app->get_ctx());
      app->p_display->print_string("%s", pin_str);
      app->p_display->print_string("Save code?");
      drawMenuItems(app);
      app->p_display->send();
    }
  };

  struct Unlock : State {
    State* clone() override final {
      return new Unlock(*this);
    }
    void enterAction(App *app) override final {
      app->set_ctx(select_classify_str);
      VoiceMsgPlay(4);
    }
    void exitAction(App *app) override final {
      app->pop_ctx();
    }
    void update(App *app) override final {
      while (check_pin(app) < 0) {
        LOGI(TAG, "PINcode is not correct");
        xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_BAD_MSK);
        VoiceMsgPlay(62);
        VoiceMsgWaitStop(portMAX_DELAY);
      }
      LOGI(TAG, "PINcode is correct");
      xEventGroupSetBits(xStatusEventGroup, STATUS_EVENT_GOOD_MSK);
      VoiceMsgPlay(61);
      VoiceMsgWaitStop(portMAX_DELAY);
      app->p_display->print_header("%s", app->get_ctx());
      app->p_display->print_string("Unlocked");
      app->p_display->send();
      VoiceMsgPlay(63);
      vTaskDelay(pdMS_TO_TICKS(5000));
      VoiceMsgPlay(64);
      VoiceMsgPlay(65);
      app->transition(new Common::Suspended(clone()));
    }
  };
} // namespace VoicePINcode

static State* getSaveDialog() {
  return new VoicePINcode::SaveDialog(
    [] (App *app) {
      app->transition(new VoicePINcode::Unlock);
    },
    &record_numbers
  );
}

void Common::initVoicePINcodeScenario(App *app) {
  app->p_display->print_header("%s", app->get_ctx());
  app->p_display->send();
  app->set_ctx("VoicePIN");
  record_numbers(app);
}
