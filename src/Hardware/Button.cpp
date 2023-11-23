#include "Button.hpp"

#include "driver/gpio.h"

#if CONFIG_IDF_TARGET_ESP32C3
#define BOOT_MODE_PIN GPIO_NUM_9
#else
#define BOOT_MODE_PIN GPIO_NUM_0
#endif

Button::Button() : IButton(BOOT_MODE_PIN) {
  gpio_reset_pin((gpio_num_t)pin_);
  gpio_set_direction((gpio_num_t)pin_, GPIO_MODE_INPUT);

  gpio_pulldown_dis((gpio_num_t)pin_);
  gpio_pullup_en((gpio_num_t)pin_);
}

bool Button::isPressed() const { return gpio_get_level((gpio_num_t)pin_) == 0; }
