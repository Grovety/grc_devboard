#pragma once
#include "IButton.hpp"

class Button : public IButton {
public:
  Button();
  bool isPressed() const override final;
};
