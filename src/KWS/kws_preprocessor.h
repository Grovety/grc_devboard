#ifndef _KWS_PREPROCESSOR_H_
#define _KWS_PREPROCESSOR_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "kws.h"

struct KWSWordDesc_t {
  size_t frame_num;
  size_t max_abs;
};

/*! \brief Global KWS output semaphore. */
extern SemaphoreHandle_t xKWSSema;
/*! \brief Global KWS word semaphore. */
extern SemaphoreHandle_t xKWSWordSema;
/*! \brief Global KWS input word queue. */
extern QueueHandle_t xKWSWordQueue;
/*! \brief Global KWS word request queue. */
extern QueueHandle_t xKWSRequestQueue;
/*! \brief Global KWS output category queue. */
extern QueueHandle_t xKWSResultQueue;
/*! \brief Global KWS word frames buffer. */
extern StreamBufferHandle_t xKWSFramesBuffer;

/*!
 * \brief Initialize KWS preprocessor.
 * \return Result.
 */
int kws_preprocessor_init();
/*!
 * \brief Request to recognize words.
 * \param req_words Number of words.
 */
void kws_req_word(size_t req_words);
/*!
 * \brief Cancel request.
 */
void kws_req_cancel();

#endif // _KWS_PREPROCESSOR_H_
