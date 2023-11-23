#pragma once

#include "IDisplay.hpp"

class Lcd : public IDisplay {
public:
  enum class Font : unsigned {
    COURB14,
    COURR14,
    F8X13B,
    F9X15,
    F9X18,
  };
  enum class Rotation : unsigned {
    PORTRAIT,
    UPSIDE_DOWN,
  };
  Lcd(Rotation rot = Rotation::UPSIDE_DOWN);

  void setFont(Font f) const;
  void setRotation(Rotation rot = Rotation::UPSIDE_DOWN) const;

  void print_status(const char *msg) override final;
  void print_header(const char *fmt, ...) override final;
  void print_string(const char *fmt, ...) override final;
  void send() override final;
  void clear() override final;

private:
  void draw_string(const char *fmt, va_list argp);
  unsigned y_offset_;
};
