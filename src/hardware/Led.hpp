#pragma once

#include "ILed.hpp"

class Led : public ILed {
public:
  Led();
  void set(bool state) override final;
  void set(Colour colour, int led_num = -1, Brightness b = _50) override final;
  void flash(Colour colour, unsigned nums = 1) override final;
};
