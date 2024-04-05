#ifndef _I2S_RX_SLOT_H_
#define _I2S_RX_SLOT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

/*! \brief Global microphone semaphore. */
extern SemaphoreHandle_t xMicSema;
/*! \brief Global microphone raw data frames ring buffer. */
extern RingbufHandle_t xMicRingBuffer;

typedef enum {
	stStdLeft = 0,
	stStdRight,
	stStdBoth,
} eSlotType;

typedef struct mic_conf_t {
  size_t sample_rate = 16000;
  eSlotType slot_type = stStdRight;
} mic_conf_t;

/*!
 * \brief Initialize microphone.
 * \param conf Config.
 */
void i2s_rx_slot_init(const mic_conf_t &conf);
/*!
 * \brief Start microphone read.
 */
void i2s_rx_slot_start();
/*!
 * \brief Stop microphone read.
 */
void i2s_rx_slot_stop();
/*!
 * \brief Release microphone.
 */
void i2s_rx_slot_release();
/*!
 * \brief Initialize microphone frames receiver.
 */
void i2s_receiver_init();

#endif // _I2S_RX_SLOT_H_
