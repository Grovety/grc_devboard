#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "ISensor.hpp"
#include "Status.hpp"

/*!
 * \brief Global read/write queues to listener.
 */
struct ListenerQueues_t {
  QueueHandle_t xTxQueue;
  QueueHandle_t xRxQueue;
} extern ListenerQueues;

#define LISTENER_STOP_MSK BIT_POS(0)
/*!
 * \brief Global bits to send signals to listener.
 */
extern EventGroupHandle_t xListenerEventGroup;
/*!
 * \brief Initialize Sensor Listener.
 * \param p_send Pointer to sensor device.
 */
void initListener(ISensor *p_sens);
