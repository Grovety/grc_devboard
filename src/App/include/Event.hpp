#pragma once

#include "IButton.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

enum eEvent : unsigned {
  NO_EVENT = 0,
  BUTTON_TO_UP,
  BUTTON_TO_DOWN,
  BUTTON_CLICK,
  BUTTON_HOLD,
  CMD_YES,
  CMD_NO,
  CMD_ONE,
  CMD_TWO,
  CMD_THREE,
  CMD_GO,
  CMD_UP,
  CMD_SHEILA,
  CMD_UNKNOWN,
};

/*!
 * \brief Global events queue.
 */
extern QueueHandle_t xEventQueue;
/*!
 * \brief Initialize Events Generator.
 * \param p_button Pointer to button device.
 */
void initEventsGenerator(IButton *p_button);
/*!
 * \brief Send event to events queue.
 * \param ev Event.
 */
void sendEvent(eEvent ev);
/*!
 * \brief Get CMD event name.
 * \return String representation.
 */
const char *decode_cmd_event(eEvent ev);
/*!
 * \brief Get CMD_NUM event as number.
 * \return Number representation.
 */
unsigned cmd_event_to_number(eEvent ev);
