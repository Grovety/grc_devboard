#include "Button.hpp"

#include "driver/gpio.h"

Button::Button(int pin) : IButton(pin) {
  gpio_reset_pin((gpio_num_t)pin_);
  gpio_set_direction((gpio_num_t)pin_, GPIO_MODE_INPUT);

  gpio_pulldown_dis((gpio_num_t)pin_);
  gpio_pullup_en((gpio_num_t)pin_);
}

bool Button::isPressed() const { return gpio_get_level((gpio_num_t)pin_) == 0; }
