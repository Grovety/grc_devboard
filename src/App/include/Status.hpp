#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "ILed.hpp"

#define GET_BIT_POS(mask) 31 - __builtin_clz(mask)
#define BIT_POS(pos) (EventBits_t(1) << pos)
#define ALL_BITS(pos) BIT_POS(pos) - 1

#define STATUS_SENSOR_LISTENER_BUSY_MSK BIT_POS(0)
#define STATUS_SENSOR_INIT_MSK BIT_POS(1)
#define STATUS_MIC_INIT_MSK BIT_POS(2)
#define STATUS_MIC_NOISY_ENV_MSK BIT_POS(3)
#define STATUS_MIC_BAD_ENV_MSK BIT_POS(4)
#define STATUS_MIC_ON_MSK BIT_POS(5)
#define STATUS_KWS_BUSY_MSK BIT_POS(6)
#define STATUS_EVENT_KWS_STOP_MSK BIT_POS(7)
#define STATUS_EVENT_GOOD_MSK BIT_POS(8)
#define STATUS_EVENT_BAD_MSK BIT_POS(9)
#define STATUS_SYSTEM_SUSPENDED_MSK BIT_POS(10)
#define STATUS_SYSTEM_BUSY_MSK BIT_POS(11)
#define STATUS_PREP_STATE_MSK BIT_POS(12)
#define STATUS_ALL_BITS_MSK ALL_BITS(13)
#define STATUS_INIT_BITS_MSK (STATUS_MIC_INIT_MSK | STATUS_SENSOR_INIT_MSK)
#define STATUS_EVENT_BITS_MSK (STATUS_EVENT_GOOD_MSK | STATUS_EVENT_BAD_MSK)
#define STATUS_MIC_ENV_BITS_MSK                                                \
  (STATUS_MIC_BAD_ENV_MSK | STATUS_MIC_NOISY_ENV_MSK)

/*!
 * \brief Global status bits.
 */
extern EventGroupHandle_t xStatusEventGroup;
/*!
 * \brief Initialize Status Monitor.
 * \param p_led Pointer to led device.
 */
void initStatusMonitor(ILed *p_led);
