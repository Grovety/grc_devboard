#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "Log.hpp"
#include "Status.hpp"

#define WAV_PLAYER_STOP_MSK BIT_POS(0)

/*! \brief Global WavPlayer events. */
extern EventGroupHandle_t xWavPlayerEventGroup;
/*! \brief Global WavPlayer queue. */
extern QueueHandle_t xWavPlayerQueue;

/*!
 * \brief Initialize WavPlayer.
 */
void initWavPlayer();