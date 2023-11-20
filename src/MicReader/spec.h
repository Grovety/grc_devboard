#ifndef _SPEC_H_
#define _SPEC_H_

/*
 * A_POLLING_TIME_MS: determines the size of the internal buffer of the driver
 *  (i.e. the max size of data that can be accumulated between consecutive reads);
 *  here we need 1 sec
 * A_FRAGMENT_TIME_MS: determines the portion of data that will be read from the driver buffer;
 *  here it is recommended to skip this parameter through the plan() function, which will determine
 *  the settings for the driver DMA and specify the size of this portion; it is recommended to read 
 *  clearly in these portions;
 *  here we can read in chunks of 10 msec (e.g.), for 1 sec is also suitable
 * A_AUDIO_DURATION_MS: defines the size of the audio buffer that is used for converting
 *  and analyzing stream data;
 */

#define KWS_DURATION_MS       1000
#define KWS_FRAME_LEN         (A_SAMPLE_RATE / 1000 * KWS_WIN_MS) //400
#define KWS_NUM_FRAMES        ((KWS_DURATION_MS - KWS_WIN_MS) / KWS_STRIDE_MS + 1) //98
#define KWS_FRAME_SHIFT       (static_cast<int16_t>(A_SAMPLE_RATE * 0.001 * KWS_STRIDE_MS)) //160

#define A_POLLING_TIME_MS     40 
#define A_FRAGMENT_TIME_MS    10 
//#define A_AUDIO_DURATION_MS   (2 * KWS_DURATION_MS + A_FRAGMENT_TIME_MS) stereo???
#define A_AUDIO_DURATION_MS   (1 * KWS_DURATION_MS + A_FRAGMENT_TIME_MS)

#endif // SPEC_H_
