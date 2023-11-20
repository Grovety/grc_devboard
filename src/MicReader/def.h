/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _DEF_H_
#define _DEF_H_

#include <vector>
#include <limits.h>
#include <stdint.h>

#ifdef ESP32
#include <driver/i2s_std.h>
#endif // ESP32

#include "scope_timer.h"
#include "frag.h"

// Board 
//#define A_TARGET_ESP32C3_XIAO               // xiao-esp32-c3
//#define A_TARGET_ESP32C3_LCD                  // ESP32-C3-0.42LCD
//#define A_TARGET_ESP32C3_MINI               // mini D1 PLUS v1.2
//#define A_TARGET_ESP32S2_MINI
#define A_TARGET_ESP32S3_LCD

// Microphone
//#define A_MIC_INMP441                         // inmp441   

// Stream
//#define A_SAMPLE_RATE                16000  // compile def; audio sample rate
#define A_SLOT_NUM                     1      
//#define A_BITS_PER_SAMPLE_DATA_8BIT         
//#define A_BITS_PER_SAMPLE_DATA_16BIT        // compile def; process sample width       
//#define A_BITS_PER_SAMPLE_DATA_24BIT
//#define A_BITS_PER_SAMPLE_DATA_32BIT

// Process
//#define A_VOLUME_X2                         // compile def; increase the volume level by 2 times      
//#define A_VOLUME_X4                         //                                        by 4 times
//#define A_VOLUME_X8                         //                                        by 8 times
//#define A_VOLUME_X16                        //                                        by 16 times
//#define A_VOLUME_X32                        //                                        by 32 times
//#define A_VOLUME_X64                        //                                        by 64 times
//#define A_VOLUME_X128                       //                                        by 128 times
//#define A_COLLECT_FRAGMENTS                 // collect fragments while detection process
#define   A_INFERENCE_POINT            0.5f

// MFCC 
//#define A_MFCC_ZERO_PREPARED_TEMPLATE         // compile def; fill the mfcc buffer with zeros
//#define A_MFCC_NOISE_PREPARED_TEMPLATE        //              fill the mfcc buffer with random values
//#define A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE  //              fill the mfcc buffer according to the environment       

// Mode
//#define A_MODE_DEBUG                          // debug mode 

#ifdef A_MODE_DEBUG
#define DEBUG_PRINTF(fmt, ...)  printf(fmt, __VA_ARGS__);
#define DEBUG_SCOPE_TIMER(c)    ScopeTimer st(c);   
#else
#define DEBUG_PRINTF(fmt, ...)  while(false) {};
#define DEBUG_SCOPE_TIMER(c)    while(false) {};
#endif

// Monitor
//#define A_MON_ENABLE                        // data print is enabled
#define A_MON_WORDS_PER_LINE           8      //
//#define A_MON_NORM_MSB_ENABLE               // msb first normalization is enabled
//#define A_MON_NORM_LSB_ENABLE               // lsb first normalization is enabled
#define A_MON_MFCC_ENABLE                     // mfcc (mels) print is enabled
#define A_MON_MFCC_WORDS_PER_LINE      8      //

// Mic interface
#if defined(A_MIC_INMP441)
// Bit width
 //#define A_HW_BITS_PER_SAMPLE_DATA_8BIT
 //#define A_HW_BITS_PER_SAMPLE_DATA_16BIT
 //#define A_HW_BITS_PER_SAMPLE_DATA_24BIT    // compile def; hw stream sample width
 //#define A_HW_BITS_PER_SAMPLE_DATA_32BIT
// Bit shift
 #define A_HW_BIT_SHIFT
#endif // A_MIC_INMP441

 #define A_BYTES_PER_SAMPLE_DATA_8BIT     ((CHAR_BIT + 7)/CHAR_BIT)
 #define A_BYTES_PER_SAMPLE_DATA_16BIT    ((CHAR_BIT + 15)/CHAR_BIT)
 #define A_BYTES_PER_SAMPLE_DATA_24BIT    ((CHAR_BIT + 23)/CHAR_BIT)
 #define A_BYTES_PER_SAMPLE_DATA_32BIT    ((CHAR_BIT + 31)/CHAR_BIT)


#if defined(A_HW_BITS_PER_SAMPLE_DATA_8BIT)
 #define A_HW_BYTES_PER_SAMPLE_DATA       A_BYTES_PER_SAMPLE_DATA_8BIT
 #define A_HW_BITS_PER_SAMPLE_DATA        8
 #define A_I2S_DATA_BIT_WIDTH             I2S_DATA_BIT_WIDTH_8BIT
 #define A_I2S_SLOT_BIT_WIDTH             I2S_SLOT_BIT_WIDTH_8BIT
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_16BIT)
 #define A_HW_BYTES_PER_SAMPLE_DATA       A_BYTES_PER_SAMPLE_DATA_16BIT
 #define A_HW_BITS_PER_SAMPLE_DATA        16
 #define A_I2S_DATA_BIT_WIDTH             I2S_DATA_BIT_WIDTH_16BIT
 #define A_I2S_SLOT_BIT_WIDTH             I2S_SLOT_BIT_WIDTH_16BIT
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_24BIT)
 #define A_HW_BYTES_PER_SAMPLE_DATA       A_BYTES_PER_SAMPLE_DATA_24BIT
 #define A_HW_BITS_PER_SAMPLE_DATA        24
 #define A_I2S_DATA_BIT_WIDTH             I2S_DATA_BIT_WIDTH_24BIT
 #define A_I2S_SLOT_BIT_WIDTH             I2S_SLOT_BIT_WIDTH_24BIT
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_32BIT)
 #define A_HW_BYTES_PER_SAMPLE_DATA       A_BYTES_PER_SAMPLE_DATA_32BIT
 #define A_HW_BITS_PER_SAMPLE_DATA        32
 #define A_I2S_DATA_BIT_WIDTH             I2S_DATA_BIT_WIDTH_32BIT
 #define A_I2S_SLOT_BIT_WIDTH             I2S_SLOT_BIT_WIDTH_32BIT
#endif 

#if defined(A_BITS_PER_SAMPLE_DATA_8BIT)
 #define A_BYTES_PER_SAMPLE_DATA          A_BYTES_PER_SAMPLE_DATA_8BIT
#elif defined(A_BITS_PER_SAMPLE_DATA_16BIT)
 #define A_BYTES_PER_SAMPLE_DATA          A_BYTES_PER_SAMPLE_DATA_16BIT
#elif defined(A_BITS_PER_SAMPLE_DATA_24BIT)
 #define A_BYTES_PER_SAMPLE_DATA          A_BYTES_PER_SAMPLE_DATA_24BIT
#elif defined(A_BITS_PER_SAMPLE_DATA_32BIT)
 #define A_BYTES_PER_SAMPLE_DATA          A_BYTES_PER_SAMPLE_DATA_32BIT
#else
 #define A_BYTES_PER_SAMPLE_DATA          A_HW_BYTES_PER_SAMPLE_DATA 
#endif 

#if A_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_8BIT
 typedef int8_t audio_t;  
#elif A_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_16BIT
 typedef int16_t audio_t;
#elif A_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_24BIT
 typedef int8_t audio_t;
#elif A_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_32BIT
 typedef int32_t audio_t;  
#else
 #error Unsupported BPS!
#endif

#if A_HW_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_8BIT
 typedef int32_t proc_t;  
#elif A_HW_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_16BIT
 typedef int32_t proc_t;
#elif A_HW_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_24BIT
 typedef int64_t proc_t;
#elif A_HW_BYTES_PER_SAMPLE_DATA == A_BYTES_PER_SAMPLE_DATA_32BIT
 typedef int64_t proc_t;  
#else
 #error Unsupported BPS!
#endif


#if defined(A_TARGET_ESP32C3_XIAO)
 #define BCK_PIN               5        // clock pin from the mic
 #define WS_PIN                4        // ws pin from the mic
 #define DATA_PIN              10       // data pin from the mic
 #define CHANNEL_SELECT_PIN    3        // pin to select the channel output from the mic
#elif defined(A_TARGET_ESP32C3_LCD)
 #define BCK_PIN               3        // clock pin from the mic
 #define WS_PIN                4        // ws pin from the mic
 #define DATA_PIN              10       // data pin from the mic
 #define CHANNEL_SELECT_PIN    1        // pin to select the channel output from the mic
#elif defined(A_TARGET_ESP32C3_MINI)
 #define BCK_PIN               8        // clock pin from the mic
 #define WS_PIN                18       // ws pin from the mic
 #define DATA_PIN              10       // data pin from the mic
 #define CHANNEL_SELECT_PIN    19       // pin to select the channel output from the mic
#elif defined(A_TARGET_ESP32S2_MINI)
 #define BCK_PIN               4        // clock pin from the mics
 #define WS_PIN                39       // ws pin from the mic
 #define DATA_PIN              5        // data pin from the mic
 #define CHANNEL_SELECT_PIN    40       // pin to select the channel output from the mic
#elif defined(A_TARGET_ESP32S3_LCD)
 #define BCK_PIN               -1       // clock pin from the mics
 #define WS_PIN                38       // ws pin from the mic
 #define DATA_PIN              45       // data pin from the mic
 #define CHANNEL_SELECT_PIN    -1       // pin to select the channel output from the mic
#endif

typedef struct DetectItem {
//    void* fragment;                     // fragment 
    size_t fragment_byte_size;          // fragment byte size
    size_t fragment_byte_offset;        // fragment byte offset
} DetectItem;

typedef struct Detection {
    size_t fragment_byte_size;          // fragment byte size
    size_t fragment_byte_offset;        // fragment byte offset
    std::vector<DetectItem>* fragments; // fragments
	int32_t environment;                // environment characteristic parameter
    size_t word_det_fragment_time_msec; // word detection fragment time interval (e.g. 10 msec)
    size_t word_det_time_msec;          // word detection time interval (e.g. 50 msec)
    size_t interword_det_time_msec;     // inter-word detection time interval (e.g. 40 msec)
    size_t before_word_time_msec;       // time interval before word (e.g. 50 msec)
    size_t syllable_det_time_msec;      // syllable detection time interval (e.g.250 msec)
} Detection;

typedef struct Environment {
    int32_t max_abs;                    // fragment max abs pcm value 
    static const int32_t max_array_size = A_DET_ENVIRONMENT_TIME_MS / 1000;
    int32_t max_abs_array[max_array_size];
} Environment;

typedef enum {
	stStdLeft = 0,
	stStdRight,   
	stStdBoth,
	stUnkn 
} eSlotType;

typedef enum {
	smMono = 0,
	smStereo   
} eSlotMode;

typedef enum {
	bwBitWidth8 = 0,
	bwBitWidth16,   
	bwBitWidth24,
	bwBitWidth32 
} eBitWidth;

typedef enum {
	bsDisable = 0,
	bsEnable   
} eBitShift;

#define PARTS(a, b) ((a + b - 1) / (b))

#endif // _DEF_H_
