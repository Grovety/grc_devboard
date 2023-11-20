#ifndef _KWS_MAIN_H_
#define _KWS_MAIN_H_

#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define A_INFERENCE_POINT 0.5f


void recognize_word(const float* src, const size_t src_len, char* dst);

#endif // _KWS_MAIN_H_
