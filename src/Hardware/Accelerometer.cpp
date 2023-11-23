#include <stdio.h>
#include <string.h>

#include "Common.hpp"
#include "I2C.hpp"

#include "driver/gpio.h"

#include <esp_err.h>
#include <esp_log.h>

#include "Accelerometer.hpp"

#define MPU_SENSOR_ADDR 0x68 // Slave address of the MPU sensor

// Register addresses of the power managment register
#define MPU_PWR_MGMT_1_REG_ADDR 0x6B
// Register addresses of the "who am I" register
#define MPU_WHO_AM_I_REG_ADDR 0x75
// Register addresses of the first data register
#define MPU_BEGIN_DATA_REG_ADDR 0x3B

#define MPU9250_SMPLRT_DIV 0x19
#define MPU9250_INT_PIN_CFG 0x37
#define MPU9250_INT_ENABLE 0x38

#define MPU9250_USER_CTRL 0x6A
#define MPU9250_CONFIG 0x1A
#define MPU9250_GYRO_CONFIG 0x1B
#define MPU9250_ACCEL_CONFIG 0x1C
#define MPU9250_ACCEL_CONFIG2 0x1D
#define MPU9250_FIFO_EN 0x23

#define WHOAMI_MPU9250 0x71
#define WHOAMI_MPU6500 0x70

static const char *TAG = "i2c-accel";

static bool readRegister(uint8_t reg_addr, uint8_t *data, size_t len) {
  return g_i2c.write_read(MPU_SENSOR_ADDR, &reg_addr, 1, data, len);
}

static bool writeRegisterByte(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  return g_i2c.write(MPU_SENSOR_ADDR, buf, sizeof(buf));
}

static float conv(const uint8_t *data) {
  int16_t res = (data[0] << 8 | data[1]);
  return res / (float)0x8001;
}

struct {
  unsigned char reg_;
  unsigned char data_;
  unsigned char delay_;
} static s_init_seq[] = {
    {MPU_PWR_MGMT_1_REG_ADDR, 0x00, 10},
    {MPU_PWR_MGMT_1_REG_ADDR, (0x01 << 7), 10}, // reset bit
    {MPU_PWR_MGMT_1_REG_ADDR, (0x01 << 0), 10},

    {MPU9250_ACCEL_CONFIG, 0x08, 1}, // AFS_4G
    {MPU9250_GYRO_CONFIG, 0x18, 1},  // GFS_2000DPS

    {MPU9250_CONFIG, 0x01, 1},
    {MPU9250_SMPLRT_DIV, 0x00, 1}, // 1000 / (srd + 1) = 1 kHz
    {MPU9250_INT_ENABLE, 0x00, 1},
    {MPU9250_ACCEL_CONFIG2, 0x00, 1},
    {MPU9250_USER_CTRL, 0x00, 1},
    {MPU9250_FIFO_EN, 0x00, 1},
    {MPU9250_INT_PIN_CFG, 0x22, 1},

    {MPU9250_INT_ENABLE, 0x01, 1}};

bool Accelerometer::open() {
  if (!g_i2c.open()) {
    ESP_LOGE(TAG, "Failed to open I2C");
    return false;
  }

#if CONFIG_IDF_TARGET_ESP32S3
  gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_6, 1);

  gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_21, 0);
#endif

  uint8_t data[2] = {0};

  bool ret = readRegister(MPU_WHO_AM_I_REG_ADDR, data, 1);
  ESP_LOGI(TAG, "WHO_AM_I = %X", data[0]);
  if (!ret || !(data[0] == WHOAMI_MPU9250 || data[0] == WHOAMI_MPU6500)) {
    ESP_LOGE(TAG, "Failed to check WHO_AM_I");
    return false;
  }

  for (unsigned i = 0; i < _countof(s_init_seq); ++i) {
    if (!writeRegisterByte(s_init_seq[i].reg_, s_init_seq[i].data_)) {
      ESP_LOGE(TAG, "Failed to write init sequence");
      return false;
    }
    delayMS(s_init_seq[i].delay_);
  }
  delayMS(100);
  return true;
}

bool Accelerometer::close() { return true; }

void Accelerometer::getData(float *buf, unsigned len) {
  uint8_t data_buf[14];
  bool ret = readRegister(MPU_BEGIN_DATA_REG_ADDR, data_buf, sizeof(data_buf));
  if (!ret) {
    ESP_LOGE(TAG, "Failed to read data");
  }

  switch (len) {
  case 6:
  default:
    buf[Acl_x] = conv(&data_buf[0]);
    buf[Acl_y] = conv(&data_buf[2]);
    buf[Acl_z] = conv(&data_buf[4]);
    buf[Vel_x] = conv(&data_buf[8]);
    buf[Vel_y] = conv(&data_buf[10]);
    buf[Vel_z] = conv(&data_buf[12]);
    break;
  case 3:
    buf[Acl_x] = conv(&data_buf[0]);
    buf[Acl_y] = conv(&data_buf[2]);
    buf[Acl_z] = conv(&data_buf[4]);
    break;
  }
}
