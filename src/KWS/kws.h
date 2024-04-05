#ifndef _KWS_MAIN_H_
#define _KWS_MAIN_H_

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#define KWS_NUM_MFCC     10
#define KWS_WIN_MS       40
#define KWS_STRIDE_MS    20
#define KWS_DURATION_MS  1000
#define KWS_FRAME_LEN    (KWS_SAMPLE_RATE / 1000 * KWS_WIN_MS)
#define KWS_FRAME_SHIFT  (KWS_SAMPLE_RATE / 1000 * KWS_STRIDE_MS)
#define KWS_FRAME_NUM    ((KWS_DURATION_MS - KWS_WIN_MS) / KWS_STRIDE_MS + 1)

#define KWS_FEATURES_LEN KWS_FRAME_NUM * KWS_NUM_MFCC

/*!
 * \brief Initialize kws model structure.
 * \return Result.
 */
bool kws_model_init_op_resolver();
/*!
 * \brief Initialize kws model.
 * \param model_ptr Pointer to model weights.
 * \param labels Pointer to labels table.
 * \param labels_num Number of labels.
 * \return Result.
 */
bool kws_model_init(const unsigned char *model_ptr, const char **labels, size_t labels_num);
/*!
 * \brief Release model.
 */
void kws_model_release();
/*!
 * \brief KWS model inference.
 * \param src MFCC coeffs.
 * \return Category.
 */
int kws_recognize_word(const float *src, size_t len);
/*!
 * \brief Get label string.
 * \param category Category index.
 * \param buffer Buffer to store label.
 * \param len Buffer len.
 */
void kws_get_category(int category, char *buffer, size_t len);

#endif // _KWS_MAIN_H_
