#ifndef _MIC_READER_H_
#define _MIC_READER_H_

#include "data_source.h"
#include "audio_rx_slot.h"

class MicReader : public DataSource {
public:

  MicReader();
  ~MicReader();

  size_t Read(void* dst, size_t bytes);
  size_t GetFragmentSize();
  size_t GetBufferSize();

  void Setup() override;
  void Close() override;
  size_t Collect(Preprocessor* pp, void** dst, int flag) override;

private:

size_t SampleProcess(void* buf, size_t bytes);

private:

  AudioRxSlot* mic_;
  IOPlan io_plan_;
  size_t fz_;
  
protected:

  audio_t* audioBuffer_;
  size_t audio_buffer_size_;
  size_t hw_fragment_size_;
  size_t kws_duration_size_;
  size_t hw_kws_duration_size_;
  int errors_;
};

#endif // _MIC_READER_H_
