#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "BaseGrc.hpp"

enum class eGrcEventId {
  NO_EVENT = 0,
  TRAIN,
  INFER,
  LOAD_WGTS,
  SAVE_WGTS,
  CLEAR_WGTS,
  LOAD_SIGNAL,
};

enum class eGrcReplyId {
  CATS_QTY = 0,
  CATEGORY,
  ERROR_VALUE,
};

struct GrcEvent_t {
  eGrcEventId id = eGrcEventId::NO_EVENT;
  void *data = nullptr;
};

struct GrcReply_t {
  eGrcReplyId id = eGrcReplyId::ERROR_VALUE;
  int data = -1;
};
/*!
 * \brief Global read/write queues to Grc Controller.
 */
struct GrcControllerQueues_t {
  QueueHandle_t xTxQueue;
  QueueHandle_t xRxQueue;
} extern GrcControllerQueues;
/*!
 * \brief Initialize Grc Controller.
 * \param p_grc Pointer to Grc application.
 */
void initGrcController(BaseGrc *p_grc);
