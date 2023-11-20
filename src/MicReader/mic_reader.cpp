#include <cstring>

#include "mic_reader.h"
#include "spec.h"

MicReader::MicReader() : fz_(0), errors_(0) {
  AudioRxSlot::plan(A_SAMPLE_RATE, A_HW_BITS_PER_SAMPLE_DATA, A_SLOT_NUM, A_POLLING_TIME_MS, A_FRAGMENT_TIME_MS, io_plan_);
  mic_ = new AudioRxSlot(BCK_PIN, WS_PIN, DATA_PIN, CHANNEL_SELECT_PIN);

  hw_fragment_size_ = GetFragmentSize();
  audio_buffer_size_ = PARTS(A_AUDIO_DURATION_MS, A_FRAGMENT_TIME_MS) * hw_fragment_size_;
  kws_duration_size_ = PARTS(A_SAMPLE_RATE * KWS_DURATION_MS, 1000) * A_BYTES_PER_SAMPLE_DATA;
  hw_kws_duration_size_ = PARTS(A_SAMPLE_RATE * KWS_DURATION_MS, 1000) * A_HW_BYTES_PER_SAMPLE_DATA;
  
  audioBuffer_ = new audio_t[PARTS(audio_buffer_size_, sizeof(audio_t))];
  if (audioBuffer_)
    std::memset(audioBuffer_, 0, audio_buffer_size_);
  else
    errors_++;
}

MicReader::~MicReader() {
  Close(); 
  if (mic_)
    delete mic_;
  if (audioBuffer_)
    delete[] audioBuffer_;
}

// Calculate the microphone interface settings and start the microphone
void MicReader::Setup() {
  DEBUG_PRINTF("dma_frame_num: %ld, dma_desc_num: %ld,  polling_buffer_size: %ld, fragment_buffer_size: %ld\n",
    io_plan_.dma_frame_num, io_plan_.dma_desc_num, io_plan_.polling_buffer_size, io_plan_.read_fragment_size);
  mic_->end();
  errors_ += mic_->begin(A_SAMPLE_RATE, io_plan_, 0, 
#if defined(A_HW_BITS_PER_SAMPLE_DATA_8BIT)
                bwBitWidth8,
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_16BIT)
                bwBitWidth16,
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_24BIT)
                bwBitWidth24,
#elif defined(A_HW_BITS_PER_SAMPLE_DATA_32BIT)
                bwBitWidth32,
#else
                #error Unsupported BPS!
#endif
                stStdRight, smMono,
#if defined(A_HW_BIT_SHIFT) 
                bsEnable
#else
                bsDisable                               
#endif                
                );
  fz_ = 0;
}

// Stop the microphone 
void MicReader::Close() {
  mic_->end();    
}

// Main stream function: read, detect, preprocess
size_t MicReader::Collect(Preprocessor* pp, void** dst, int flag) {
  if (!pp) {
    fz_ = Read(reinterpret_cast<uint8_t *>(audioBuffer_), hw_kws_duration_size_);
    return fz_;
  } // !pp

  if (!dst) 
    return 0; 
  if (!fz_)
    return 0;

  { // Process
    DEBUG_SCOPE_TIMER("Process");
    fz_ = SampleProcess(audioBuffer_, fz_);
  }
#ifdef A_MON_ENABLE
  mon(audioBuffer_, fz_, A_BYTES_PER_SAMPLE_DATA);
#endif // A_MON_ENABLE
  size_t apply = 0;
  { // MFCC
    DEBUG_SCOPE_TIMER("MFCC");
    apply = pp->Apply(audioBuffer_, kws_duration_size_, *dst, 0);
  } // MFCC
  fz_ = 0;
  return apply;
}

size_t MicReader::Read(void* dst, size_t bytes) {
  size_t sz = 0;
  if (dst && bytes) 
    while (bytes >= (sz + io_plan_.read_fragment_size))
      sz += mic_->read(reinterpret_cast<uint8_t *>(dst) + sz, io_plan_.read_fragment_size);
  return sz;
}

// Return the desired size of the audio stream reading transaction
size_t MicReader::GetFragmentSize() {
  return io_plan_.read_fragment_size;
}

// Return microphone interface driver buffer size
size_t MicReader::GetBufferSize() {
  return io_plan_.polling_buffer_size;
}

/*! 
 * \brief Convert a stream
 *
 * \param buf[in] buffer of analyzed stream data
 * \param bytes[in] buffer size
 * \return conversion stream data buffer size
 */
size_t MicReader::SampleProcess(void* buf, size_t bytes) {
#if A_BYTES_PER_SAMPLE_DATA > A_HW_BYTES_PER_SAMPLE_DATA
#error Unsupported BPS!
#endif

#define SP_DATA_OFFSET   (A_HW_BYTES_PER_SAMPLE_DATA - A_BYTES_PER_SAMPLE_DATA)

#if defined(A_VOLUME_X2)
#define SP_VOLUME_SHT  1
#define SP_VOLUME_MSK  0x40
#elif defined(A_VOLUME_X4)
#define SP_VOLUME_SHT  2
#define SP_VOLUME_MSK  0x60
#elif defined(A_VOLUME_X8)
#define SP_VOLUME_SHT  3
#define SP_VOLUME_MSK  0x70
#elif defined(A_VOLUME_X16)
#define SP_VOLUME_SHT  4
#define SP_VOLUME_MSK  0x78
#elif defined(A_VOLUME_X32)
#define SP_VOLUME_SHT  5
#define SP_VOLUME_MSK  0x7C
#elif defined(A_VOLUME_X64)
#define SP_VOLUME_SHT  6
#define SP_VOLUME_MSK  0x7E
#elif defined(A_VOLUME_X128)
#define SP_VOLUME_SHT  7
#define SP_VOLUME_MSK  0x7F
#else
#define SP_VOLUME_SHT  0
#define SP_VOLUME_MSK  0x00
#endif

  if ((A_BYTES_PER_SAMPLE_DATA == A_HW_BYTES_PER_SAMPLE_DATA) && (SP_VOLUME_SHT == 0))
    return bytes;

  uint8_t* s = reinterpret_cast<uint8_t*>(buf);
  uint8_t* d = reinterpret_cast<uint8_t*>(buf);
  const size_t samples = bytes / A_HW_BYTES_PER_SAMPLE_DATA;

  if (SP_VOLUME_SHT == 0) {
    // No sample reference level increase
    for (size_t z = 0; z < samples; z++) {
      s += SP_DATA_OFFSET;
      for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++) {
        *d++ = *s++;
      }
    }
    return samples * A_BYTES_PER_SAMPLE_DATA;
  }
  // Sample reference level increase
  uint32_t val;
  int sat;
  for (size_t z = 0; z < samples; z++) {
    // Collect, convert and increase the sample reference value
    sat = 0;
    if (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & 0x80) {
      if ((s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & SP_VOLUME_MSK) != SP_VOLUME_MSK) {
        sat = 1;
        for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++)
          d[i] = 0;
        d[A_BYTES_PER_SAMPLE_DATA - 1] |= 0x80;
      }
    }
    else if (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & SP_VOLUME_MSK) {
      sat = 1;
      for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++)
        d[i] = 0xFF;
      d[A_BYTES_PER_SAMPLE_DATA - 1] &= 0x7F;
    }
    if (!sat) {
      val = s[0] & 0xFF;
#if A_HW_BYTES_PER_SAMPLE_DATA == 2
      val |= s[1] << 8;
#elif A_HW_BYTES_PER_SAMPLE_DATA == 3
      val |= ((s[2] << 8) | s[1]) << 8;
#elif A_HW_BYTES_PER_SAMPLE_DATA == 4
      val |= ((((s[3] << 8) | s[2]) << 8) | s[1]) << 8;
#endif
      val <<= SP_VOLUME_SHT;
      val >>= (SP_DATA_OFFSET * 8);
      for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++) {
        d[i] = static_cast<uint8_t>(val);
        val >>= 8;
      }
      if (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & 0x80)
        d[A_BYTES_PER_SAMPLE_DATA - 1] |= 0x80;
      else
        d[A_BYTES_PER_SAMPLE_DATA - 1] &= 0x7F;
    }
    d += A_BYTES_PER_SAMPLE_DATA;
    s += A_HW_BYTES_PER_SAMPLE_DATA;
  } // for
  return samples * A_BYTES_PER_SAMPLE_DATA;
}
