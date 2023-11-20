#ifndef _MFCC_PREPROCESSOR_H_
#define _MFCC_PREPROCESSOR_H_

#include "spec.h"
#include "data_source.h"
#include "mfcc.h"

class MfccPreprocessor : public Preprocessor {
public:

  MfccPreprocessor();
  ~MfccPreprocessor();

  size_t Apply(const void* src, const size_t num_samples, void* dst, const int n_frames) override;
  int Errors() override;

private:

  MFCC* mfcc_;
  float* mfccBufferTemplate_;
  int errors_;
};

#endif // _MFCC_PREPROCESSOR_H_
