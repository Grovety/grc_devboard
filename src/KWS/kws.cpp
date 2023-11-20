#include <tvm_runtime.h>
#include <tvmgen_kws.h>

#include "kws.h"
#include "model_tensors.h"
#include "mfcc_tmp.h"


#if defined(KWS_INPUT_SCALE) && defined(KWS_INPUT_ZERO)
void quantize_model_input_int8(const float *src, int8_t *dst) {
	for (int i = 0; i < kws_input_len; i++)
		dst[i] = (int8_t)(src[i] / KWS_INPUT_SCALE + KWS_INPUT_ZERO);
}
#endif


#if defined(KWS_OUTPUT_SCALE) && defined(KWS_OUTPUT_ZERO)
void quantize_model_output_int8(const int8_t *src, float *dst) {
	for (int i = 0; i < kws_output_len; i++)
		dst[i] = (src[i] - KWS_OUTPUT_ZERO) * KWS_OUTPUT_SCALE;
}
#endif


void set_model_input(const float *src, const size_t src_size, float *dst) {
    memcpy(dst, src, sizeof(float) * src_size);
    for (size_t i = src_size; i < kws_input_len; i++)
        dst[i] = mfcc_tmp_frame[i % mfcc_tmp_frame_len];
}


void model_inference(float* src, char* dst) {
    const char* categories[] = {"_silence_", "_unknown_", "yes", "no", "go", "stop", "one", "two", "three", "four", "five", "sheila", "up"};

    size_t f = 0;
    for (size_t i = 0; i < kws_output_len; i++) {
        if (src[i] > src[f])
            f = i;
    }

    if (src[f] >= A_INFERENCE_POINT)
        strcpy(dst, categories[f]);
    else
        strcpy(dst, "_unknown_");
}


void recognize_word(const float* src, const size_t src_len, char* dst) {
    // set input
#if defined(KWS_INPUT_SCALE) && defined(KWS_INPUT_ZERO)
	float *in_buffer = new float[kws_input_len];
    set_model_input(src, src_len, in_buffer);
	quantize_model_input_int8(in_buffer, kws_input);
	delete [] in_buffer;
#else
    set_model_input(src, src_len, (float*)kws_input);
#endif

    // run model
    tvmgen_kws_run(&kws_inputs, &kws_outputs);

    // get inference
#if defined(KWS_OUTPUT_SCALE) && defined(KWS_OUTPUT_ZERO)
	float *out_buffer = new float[kws_output_len];
	quantize_model_output_int8(kws_output, out_buffer);
    model_inference(out_buffer, dst);
	delete [] out_buffer;
#else
    model_inference((float*)kws_output, dst);
#endif
}
