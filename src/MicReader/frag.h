#ifndef _FRAG_H_
#define _FRAG_H_

#include "spec.h"

/*
 * Two ways of preparing MFCC coefficients before shifts are proposed:
 * 1. coefficients are recalculated for each phrase (any A_MFCC_xx_PREPARED_TEMPLATE flags are not defined;
 *    the microphone stream is read in chunks of 1000 msec)
 * 2. coefficients are prepared only 1 time before the start of the main procedure  cycle
 *    (see the A_MFCC_ZERO_PREPARED_TEMPLATE flag (filled with zeros), or the A_MFCC_NOISE_PREPARED_TEMPLATE flag
 *    (filled with random data), or the A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE flag (calculated based on the real noise
 *    of the environment; the microphone stream is taken in portions of 500 msec).
 */

/*
 * A_STRIDE_MS: determines the size of the stride of the audio buffer data
 *  (not to be confused with the stride of the MFCC coefficients calculation procedure)
 * A_INIT_STRIDE_MS: determines the size of the first stride of the audio buffer data
 * A_STRIDED_DURATION_MS: determines the boundary for the last stride of the audio buffer data
 */

#define A_STRIDE_MS           KWS_WIN_MS 
#define A_INIT_STRIDE_MS      (5 * A_STRIDE_MS)
#define A_STRIDED_DURATION_MS (A_INIT_STRIDE_MS + 2 * A_STRIDE_MS)

/*
 * A_DET_START_TIME_MS: the time to start and turn on the audio interface
 * A_DET_ENVIRONMENT_TIME_MS: the time interval for analyzing the mic environment (silence)
 * A_DET_WORD_FRAGMENT_TIME_MS: the time interval by which the cumulative values
 *  of the flow characteristics are calculated
 * A_DET_WORD_TIME_MS: the time interval by which the definition of the beginning
 *  of a word is performed
 * A_DET_INTERWORD_TIME_MS: the time interval by which the end of the word is determined
 * A_DET_BEFORE_WORD_MS: the interval of 'silence' before the word for the inference procedure
 * A_DET_SYLLABLE_TIME_MS: estimated syllable duration
 */

#define A_DET_START_TIME_MS           1000
#define A_DET_ENVIRONMENT_TIME_MS     3000
#define A_DET_WORD_FRAGMENT_TIME_MS   10
#define A_DET_WORD_TIME_MS            50
#define A_DET_BEFORE_WORD_MS          50
#define A_DET_INTERWORD_TIME_MS       250
#define A_DET_SYLLABLE_TIME_MS        250

#endif // _FRAG_H_

