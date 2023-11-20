#ifndef _FRAGMENT_DETECTOR_H_
#define _FRAGMENT_DETECTOR_H_

#include "def.h"

class FragmentDetector {
  const proc_t d0 = proc_t(0);

public:

  FragmentDetector(Detection& det);
  ~FragmentDetector();
  
  void Init(const size_t samples, const size_t bytes_width);
  void Detect(const int32_t value);

private:

  Detection& det_;
  size_t bytes_width_;
  size_t samples_;
  size_t samples_per_fragment_det_;
  size_t samples_per_before_det_;
  size_t fragments_per_word_det_;
  size_t fragments_per_interword_det_;
  size_t fragments_per_syllable_det_;
  proc_t fragment_trait_;
  size_t fragment_samples_;
  float det_trait_;
  size_t det_sample_num_;
  size_t det_sample_cnt_;
  size_t word_fragments_;
  size_t interword_fragments_;
  proc_t word_trait_;
  size_t word_sample_num_;
  size_t sample_num_;
};

#endif // _FRAGMENT_DETECTOR_H_
