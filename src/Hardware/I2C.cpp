#include <string.h>

#include "I2C.hpp"

#include <driver/i2c.h>
#include <esp_log.h>

#define I2C_MASTER_SDA_IO 3
#define I2C_MASTER_SCL_IO 4

I2C g_i2c(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);

#define I2C_MASTER_NUM I2C_NUM_1
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "i2c";

bool I2C::open() {
  if (isOpened_)
    return true;

  i2c_port_t i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf;
  memset(&conf, '\0', sizeof(conf));
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = sda_;
  conf.scl_io_num = scl_;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

  i2c_param_config(i2c_master_port, &conf);

  esp_err_t res =
      i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE,
                         I2C_MASTER_TX_BUF_DISABLE, 0);
  ESP_ERROR_CHECK(res);
  ESP_LOGI(TAG, "I2C initialized successfully");

  isOpened_ = true;
  return res == ESP_OK;
}

bool I2C::close() {
  if (!isOpened_)
    return true;

  esp_err_t res = i2c_driver_delete(I2C_MASTER_NUM);
  ESP_ERROR_CHECK(res);
  ESP_LOGI(TAG, "I2C de-initialized successfully");

  isOpened_ = false;
  return res == ESP_OK;
}

bool I2C::write(uint8_t dev_adr, const uint8_t *buf, size_t size) {
  if (!isOpened_)
    return false;

  esp_err_t ret =
      i2c_master_write_to_device(I2C_MASTER_NUM, dev_adr, buf, size,
                                 I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  return ret == ESP_OK;
}

bool I2C::write_read(uint8_t dev_adr, const uint8_t *wbuf, size_t wsize,
                     uint8_t *rbuf, size_t rsize) {
  if (!isOpened_)
    return false;

  esp_err_t ret = i2c_master_write_read_device(
      I2C_MASTER_NUM, dev_adr, wbuf, wsize, rbuf, rsize,
      I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  return ret == ESP_OK;
}
