#pragma once
#include <stdint.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_NONE 3
#define _LOG_LEVEL_NUM 4

#if USE_LOG

void _log(int8_t level, const char *tag, const char *fmt, ...);
void _log(const char *fmt, ...);

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOGD(tag, ...) _log(LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#else
#define LOGD(tag, ...)                                                         \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOGI(tag, ...) _log(LOG_LEVEL_INFO, tag, __VA_ARGS__)
#else
#define LOGI(tag, ...)                                                         \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOGE(tag, ...) _log(LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#else
#define LOGE(tag, ...)                                                         \
  do {                                                                         \
  } while (0)
#endif

#if LOG_LEVEL < LOG_LEVEL_NONE
#define LOG(...) _log(__VA_ARGS__)
#else
#define LOG(...)                                                               \
  do {                                                                         \
  } while (0)
#endif

#else /*USE_LOG*/

#define LOGD(...)                                                              \
  do {                                                                         \
  } while (0)
#define LOGI(...)                                                              \
  do {                                                                         \
  } while (0)
#define LOGE(...)                                                              \
  do {                                                                         \
  } while (0)
#define LOG(...)                                                               \
  do {                                                                         \
  } while (0)

#endif /*USE_LOG*/
