#include  <algorithm>
#include  <cstring>

#include "def.h"
#include "utils.h"
#include "mfcc_preprocessor.h"

MfccPreprocessor::MfccPreprocessor() : mfccBufferTemplate_(nullptr), errors_(0) {
  if (!(mfcc_ = new MFCC(KWS_NUM_MFCC, KWS_FRAME_LEN)))
    errors_++;
#if defined(A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE) || defined(A_MFCC_ZERO_PREPARED_TEMPLATE) || defined(A_MFCC_NOISE_PREPARED_TEMPLATE)
 #if GET_MEL_SPEC
  mfccBufferTemplate_ = new float[NUM_FBANK_BINS * KWS_NUM_FRAMES];
 #else
  mfccBufferTemplate_ = new float[KWS_NUM_MFCC * KWS_NUM_FRAMES];
 #endif
  if (!mfccBufferTemplate_)
    errors_++;
#endif
}

MfccPreprocessor::~MfccPreprocessor() {
  if (mfccBufferTemplate_) 
    delete[] mfccBufferTemplate_;
  if (mfcc_)  
    delete mfcc_;
}

// Push and calculate MFCC for the pushed data
size_t MfccPreprocessor::Apply(const void* src, const size_t num_samples, void* dst, const int n_frames) {
  if (!num_samples)
    return 0;

  float* buf;
  const size_t new_frames = std::min(static_cast<size_t>(KWS_NUM_FRAMES),
    static_cast<size_t>(((num_samples) - KWS_FRAME_LEN) / KWS_FRAME_SHIFT + 1));
  const size_t old_frames = KWS_NUM_FRAMES - new_frames;

#if GET_MEL_SPEC  
  const size_t num_per_frame = NUM_FBANK_BINS;
#else
  const size_t num_per_frame = KWS_NUM_MFCC;
#endif  

  if (!dst) {
    if (!mfccBufferTemplate_)
      return 0;
    buf = reinterpret_cast<float*>(mfccBufferTemplate_);
    std::memmove(buf, buf + new_frames * num_per_frame, old_frames * num_per_frame * sizeof(*buf));  
#if defined(A_MFCC_ZERO_PREPARED_TEMPLATE)      
    for(int f = old_frames * num_per_frame; f < KWS_NUM_FRAMES * num_per_frame; f++)
      buf[f] = 0.0f;
    return new_frames * num_per_frame;
#elif defined(A_MFCC_NOISE_PREPARED_TEMPLATE)        
    get_random_data(buf + old_frames * num_per_frame, -1.0f, 1.0f, new_frames * num_per_frame);
    return new_frames * num_per_frame;
#endif
  }
  else {
    buf = reinterpret_cast<float*>(dst);
    std::memmove(buf, buf + new_frames * num_per_frame, old_frames * num_per_frame * sizeof(*buf));  
  }

  if (src) {  
    for (int f = old_frames; f < KWS_NUM_FRAMES; f++)
      mfcc_->MfccCompute(reinterpret_cast<const audio_t*>(src) + (f - old_frames) * KWS_FRAME_SHIFT, buf + f * num_per_frame);
  }
  else {
    if (dst && mfccBufferTemplate_)
      std::memmove(reinterpret_cast<float*>(dst) + old_frames * num_per_frame, 
        reinterpret_cast<float*>(mfccBufferTemplate_) + old_frames * num_per_frame, new_frames * num_per_frame * sizeof(float));
  }
  return new_frames * num_per_frame;
}

int MfccPreprocessor::Errors() {
  return errors_;
}