#include "driver/gpio.h"
#include "driver/i2c.h"
#include "grc/drivers/grc_ll_i2c.h"
#include "grc/grc.h"

#include "IGrc.hpp"
#include "common.hpp"

#include <string.h>

constexpr char TAG[] = "IGrc";

// TODO: merge with other i2cs
#define GRC_CONFIG_I2C_MASTER_SDA GPIO_NUM_12
#define GRC_CONFIG_I2C_MASTER_SCL GPIO_NUM_13
#define GRC_CONFIG_DATA_READY_PIN_SDA GPIO_NUM_14

#define GRC_CONFIG_I2c_PORT I2C_NUM_0

static struct grc_ll_i2c_dev ll_dev = {.type = PROTOCOL_INTERFACE_I2C,
                                       .sda_io_num = GRC_CONFIG_I2C_MASTER_SDA,
                                       .scl_io_num = GRC_CONFIG_I2C_MASTER_SCL,
                                       .data_ready_io_num =
                                           GRC_CONFIG_DATA_READY_PIN_SDA,
                                       .i2c_num = GRC_CONFIG_I2c_PORT,
                                       .clk_speed = 400000,
                                       .slave_addr = 0x36,
                                       .timeout_us = 1000 / portTICK_PERIOD_MS};

static struct grc_device grc_dev = {.ll_dev = &ll_dev, .version = 1};

int IGrc::initReq(const HP &hp) const {
  LOGI(TAG, "init GRC");
  LOGI(TAG, "grc fix download 1");

  struct grc_config conf = {.arch = uint32_t(ARCH_CONSTRUCTOR(0, hp.m_InputComponents, hp.m_ReservoirNeurons, 0))};
  int res = grc_init(&grc_dev, &conf);
  if (res < 0) {
    LOGE(TAG, "initialize grc failed %d", res);
    return res;
  }

  int config_len = 6;
  struct hp_setup config[config_len] = {
      hp_setup{.type = PREDICT_SIGNAL,
               .value = (hp.m_PredictSignal) ? 1.0f : 0.0f},
      hp_setup{.type = SEPARATE_INACCURACIES,
               .value = (hp.m_SeparateInaccuracies) ? 1.0f : 0.0f},
      hp_setup{.type = NOISE, .value = (float)hp.m_Noise},
      hp_setup{.type = INPUT_SCALING, .value = (float)hp.m_InputScaling},
      hp_setup{.type = FEEDBACK_SCALING, .value = (float)hp.m_FeedbackScaling},
      hp_setup{.type = THRESHOLD_FACTOR, .value = (float)hp.m_ThresholdFactor}};
  res = grc_set_config(&grc_dev, config, config_len);
  if (res < 0) {
    LOGE(TAG, "failed to set hyperparams %d", res);
  }
  return res;
}

int IGrc::clearReq() const {
  LOGI(TAG, "clear Grc state");
  int res = grc_clear_state(&grc_dev);
  if (res < 0) {
    LOGE(TAG, "failed to clear Grc state");
  }
  return res;
}

int IGrc::trainReq(unsigned len, const float *vals, int category) const {
  LOGI(TAG, "train, category=%d", category);

  struct grc_training_params training_params = {};
  if (category >= 0) {
    training_params.flags = GRC_PARAMS_OVERWRITE;
    training_params.tag = category;
  } else {
    training_params.flags = GRC_PARAMS_ADD_NEW_TAG;
  }
  int train_category = grc_train(&grc_dev, &training_params, vals, len);
  if (train_category < 0) {
    LOGE(TAG, "failed grc_train %d", train_category);
  }
  return train_category;
}

int IGrc::inferenceReq(unsigned len, const float *vals, int category) const {
  LOGI(TAG, "inference, category=%d", category);
  struct grc_inference_params inf_params = {};
  if (category >= 0) {
    inf_params.flags = GRC_PARAMS_SINGLE_CLASS;
    inf_params.tag = category;
  }
  int inf_category = grc_inference(&grc_dev, &inf_params, vals, len);
  return inf_category;
}

unsigned IGrc::getQtyReq() const {
  int qty = grc_get_classes_number(&grc_dev);
  if (qty < 0) {
    LOGE(TAG, "failed grc_get_classes_number %d", qty);
  }
  return qty;
}

unsigned IGrc::saveTrainDataReq(std::vector<RT> &data) const {
  LOGI(TAG, "extract train meta data");

  grc_internal_state state = {};
  struct grc_internal_state states[] = {state};
  uint32_t len;
  int res = grc_download(&grc_dev, states, &len);
  if (res > 0) {
    data.clear();
    int total_data = 0;
    for (int j = 0; j < len; j++) {
      total_data += states[j].len;
    }
    if (total_data > 0 && total_data < (1 << 15)) {
      data.reserve(total_data);

      for (int j = 0; j < len; j++) {
        for (int i = 0; i < states[j].len; i++) {
          data.push_back(states[j].values[i]);
        }
      }
    }
    for (int j = 0; j < len; j++) {
      if (states[j].values != nullptr) {
        free(states[j].values);
      }
    }
  } else {
    LOGE(TAG, "Failed grc_download");
  }
  return res;
}

bool IGrc::loadTrainDataReq(unsigned qty, unsigned len, const RT *vals) const {
  struct grc_internal_state states[1] = {
      grc_internal_state{.tag = 0, .len = len, .values = const_cast<RT *>(vals)}};
  LOGI(TAG, "load train metadata");

  const auto ret = grc_upload(&grc_dev, states, qty);
  if (ret < 0) {
    LOGE(TAG, "%s failed %d", __FUNCTION__, ret);
    return false;
  }
  return true;
}
