#pragma once
#include "IButton.hpp"

class Button : public IButton {
public:
  Button(int pin);
  bool isPressed() const override final;
};
