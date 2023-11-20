#ifndef _MODEL_TENSORS_H_
#define _MODEL_TENSORS_H_

#include <stdlib.h>
#include <stdio.h>


// allocate input tensor
const size_t kws_input_bytes = MODEL_IN_BYTES;
const size_t kws_input_len = 490;
__attribute__((section(".data.tvm"), aligned(32))) int8_t kws_input[kws_input_len * kws_input_bytes];
struct tvmgen_kws_inputs kws_inputs = {.input = kws_input,};


// allocate output tensor
const size_t kws_output_bytes = MODEL_OUT_BYTES;
const size_t kws_output_len = 13;
__attribute__((section(".data.tvm"), aligned(32))) int8_t kws_output[kws_output_len * kws_output_bytes];
struct tvmgen_kws_outputs kws_outputs = {.Identity = kws_output,};

#endif // _MODEL_TENSORS_H_