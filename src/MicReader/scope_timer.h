#ifndef _SCOPE_TiMER_H_
#define _SCOPE_TiMER_H_

#include "esp_timer.h"

class ScopeTimer {
  const char* msg;
  int64_t t1, t2;

  public:
  ScopeTimer(const char* msg) : msg(msg) {
    t1 = esp_timer_get_time();
  }
  ~ScopeTimer() {
    t2 = esp_timer_get_time();
    printf("%s: %lld us", msg, t2 - t1);
  }
};

#define LOG_TIME(x, name)    \
  t1 = esp_timer_get_time(); \
  x;                         \
  t2 = esp_timer_get_time(); \
  printf("%s: %lld us", name, t2 - t1);

#endif // _SCOPE_TiMER_H_
