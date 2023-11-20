#ifndef _MIC_FRAGMENT_READER_H_
#define _MIC_FRAGMENT_READER_H_

#include "mic_reader.h"

class MicFragmentReader : private MicReader {
public:    

  MicFragmentReader();
  ~MicFragmentReader();

  void Setup() override;
  void Close() override;
  int Errors() override;
  size_t Collect(Preprocessor* pp, void** dst, int flag) override;
  void EnvironmentArrayRightshift();
  
  double GetFragTime() const { return frag_time_; };
  size_t GetEnvironment() const { return det_.environment; };

private:
  void EnvironmentProcess(void* buf, const size_t bytes, const size_t bytes_width, Environment& env, Detection& det);
  size_t DetectionProcess(void* buf, const size_t bytes, const size_t bytes_width, Detection* det);
  size_t SampleProcess(void* buf, size_t bytes, Detection* det = nullptr);
  void AverageSigProcess(void* buf, const size_t bytes, const size_t bytes_width);

private:

  Environment env_;
  Detection det_;
  double frag_time_=0;
  int stage_;
  size_t sz_;
  size_t fz_;
  size_t oft_;
  size_t oft_e_;
  size_t oft_l_;
  
  size_t audio_buffer_stride_size_;
  size_t audio_buffer_first_stride_size_;
  size_t audio_buffer_strided_oft_;
  size_t hw_kws_half_duration_size_;
  size_t hw_kws_dbl_duration_size_;
};

#endif // _MIC_FRAGMENT_READER_H_
