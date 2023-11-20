#pragma once

#include "common.hpp"
#include "driver/gpio.h"

inline void c3_reset() {
  gpio_set_direction((gpio_num_t)GPIO_NUM_11, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)GPIO_NUM_11, 0);
  delayMS(100);
  gpio_set_level((gpio_num_t)GPIO_NUM_11, 1);
  gpio_pulldown_dis((gpio_num_t)GPIO_NUM_11);
  gpio_pullup_en((gpio_num_t)GPIO_NUM_11);
  delayMS(1000);
}
