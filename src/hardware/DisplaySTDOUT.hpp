#pragma once

#include "IDisplay.hpp"
#include "stdio.h"

class DisplaySTDOUT : public IDisplay {
public:
  void print_status(const char *msg) override final { printf("(%s)\n", msg); }
  void print_header(const char *fmt, ...) override final {
    va_list argp;
    va_start(argp, fmt);

    vprintf(fmt, argp);
    printf("\n");

    va_end(argp);
  }
  void print_string(const char *fmt, ...) override final {
    va_list argp;
    va_start(argp, fmt);

    vprintf(fmt, argp);
    printf("\n");

    va_end(argp);
  }
  void send() override final {}
  void clear() override final {}
};
