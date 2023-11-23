#include "BaseGrc.hpp"

#include "grc/drivers/esp32/grc_esp32_impl.h"

#define GRC_CONFIG_I2C_MASTER_SDA GPIO_NUM_13
#define GRC_CONFIG_I2C_MASTER_SCL GPIO_NUM_12
#define GRC_CONFIG_DATA_READY_PIN_SDA GPIO_NUM_14

#define GRC_CONFIG_I2C_PORT I2C_NUM_0

#define GRC_CONFIG_GRC_RESET_PIN GPIO_NUM_11

static grc_ll_i2c_dev_esp32 ll_dev = {.type = PROTOCOL_INTERFACE_I2C_ESP32,
                                      .sda_io_num = GRC_CONFIG_I2C_MASTER_SDA,
                                      .scl_io_num = GRC_CONFIG_I2C_MASTER_SCL,
                                      .data_ready_io_num =
                                          GRC_CONFIG_DATA_READY_PIN_SDA,
                                      .reset_io_num = GRC_CONFIG_GRC_RESET_PIN,
                                      .i2c_num = GRC_CONFIG_I2C_PORT,
                                      .clk_speed = 400000,
                                      .slave_addr = 0x36,
                                      .timeout_us = 1000 / portTICK_PERIOD_MS};

BaseGrc::BaseGrc(const char *name) : grc_(&ll_dev), name_(name) {
  grc_.reset();
}

BaseGrc::~BaseGrc() { grc_.clearState(); }

int BaseGrc::init(HP hp) {
  int res = grc_.init(hp);
  if (res < 0) {
    LOGE(name_, "failed to initialize Grc, %d", res);
  }
  return res;
}

int BaseGrc::clear() {
  int res = grc_.clearState();
  if (res < 0) {
    LOGE(name_, "failed to clear Grc state, %d", res);
  }
  return res;
}

unsigned BaseGrc::save(std::vector<RT> &data) {
  LOGD(name_, __FUNCTION__);
  int res = grc_.save(data);
  if (res < 0) {
    LOGE(name_, "failed to save Grc state, %d", res);
    return 0;
  }
  return res;
}

bool BaseGrc::load(unsigned qty, const std::vector<RT> &data) {
  LOGD(name_, __FUNCTION__);
  int res = grc_.load(qty, data.size(), data.data());
  if (res < 0) {
    LOGE(name_, "failed to load Grc state, %d", res);
    return false;
  }
  return true;
}

unsigned BaseGrc::getQty() const {
  int res = grc_.getQty();
  if (res < 0) {
    LOGE(name_, "failed to get number of trained categories, %d", res);
    return 0;
  }
  return res;
}

const char *BaseGrc::getName() const { return name_; }
