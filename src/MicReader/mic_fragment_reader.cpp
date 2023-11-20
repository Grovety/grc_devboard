#include <cstring>
#include <cmath>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "frag.h"
#include "scope_timer.h"
#include "fragment_detector.h"
#include "mic_fragment_reader.h"


MicFragmentReader::MicFragmentReader()
  : stage_(0), sz_(0), fz_(0), oft_(0), oft_e_(0), oft_l_(0) {
  audio_buffer_stride_size_ = PARTS(A_SAMPLE_RATE * A_STRIDE_MS, 1000) * A_BYTES_PER_SAMPLE_DATA;
  audio_buffer_first_stride_size_ = PARTS(A_SAMPLE_RATE * A_INIT_STRIDE_MS, 1000) * A_BYTES_PER_SAMPLE_DATA;
  audio_buffer_strided_oft_ = PARTS(PARTS(A_SAMPLE_RATE * A_STRIDED_DURATION_MS, 1000) * A_BYTES_PER_SAMPLE_DATA, 
    audio_buffer_stride_size_) * audio_buffer_stride_size_;
  hw_kws_half_duration_size_ = PARTS(A_SAMPLE_RATE * KWS_DURATION_MS, 2 * 1000) * A_HW_BYTES_PER_SAMPLE_DATA;
  hw_kws_dbl_duration_size_ = hw_kws_duration_size_ * 2;

  det_.word_det_fragment_time_msec = A_DET_WORD_FRAGMENT_TIME_MS;
  det_.word_det_time_msec          = A_DET_WORD_TIME_MS;
  det_.interword_det_time_msec     = A_DET_INTERWORD_TIME_MS;
  det_.before_word_time_msec       = A_DET_BEFORE_WORD_MS;
  det_.syllable_det_time_msec      = A_DET_SYLLABLE_TIME_MS;
#ifdef A_COLLECT_FRAGMENTS  
  if (!(det_.fragments = new std::vector<DetectItem>()))
    errors_++;
#else 
  det_.fragments = nullptr;
#endif // A_COLLECT_FRAGMENTS
}

MicFragmentReader::~MicFragmentReader() {
  Close();
}

// Configure and launch audio capture 
void MicFragmentReader::Setup() {
  MicReader::Setup();
  if (errors_)
    return;
 
  sz_ = 0;
  fz_ = 0;
  oft_ = 0;
  oft_e_ = 0; 
  oft_l_ = 0;
  stage_ = 0;

  { // Start 
    DEBUG_SCOPE_TIMER("Start (Setup)");
    const size_t start_size = PARTS(A_SAMPLE_RATE * A_DET_START_TIME_MS, 1000) * A_HW_BYTES_PER_SAMPLE_DATA;
    size_t start_sz = 0;
    while (start_sz < start_size)
      start_sz += Read(audioBuffer_, hw_fragment_size_);
  }
  vTaskDelay(1);

  // Init envieroment buffer
  for (int i = 0; i < env_.max_array_size; i++)
    env_.max_abs_array[i] = 200;
}

void MicFragmentReader::Close() {
  MicReader::Close();
}

int MicFragmentReader::Errors() {
  return errors_;
}

// Main stream function: read, detect, preprocess
size_t MicFragmentReader::Collect(Preprocessor* pp, void** dst, int flag) {
  double time_start_ = esp_timer_get_time();
#if defined(A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE) || defined(A_MFCC_ZERO_PREPARED_TEMPLATE) || defined(A_MFCC_NOISE_PREPARED_TEMPLATE)    
  const bool btemplate = true;
#else
  const bool btemplate = false;
#endif // A_MFCC_x_TEMPLATE 

  if (!pp) {
	frag_time_ = 0;
    det_.fragment_byte_size = 0;
    if (btemplate) {
      do {
        size_t kws_sz = 0;
        size_t read_size = std::min(hw_kws_duration_size_ - sz_, hw_kws_half_duration_size_);
        while((sz_ <= audio_buffer_size_ - hw_fragment_size_) && (kws_sz < read_size)) {
          size_t bytes = Read(reinterpret_cast<uint8_t *>(audioBuffer_) + sz_, hw_fragment_size_);
          kws_sz += bytes;
          sz_ += bytes;
        }

        if (det_.fragment_byte_size) {
          if (sz_ >= hw_kws_duration_size_) {
            // DEBUG_PRINTF("[break_2]\n");
            break;
          }
          continue;
        }
        
        // size_t samples_count = (sz_ / A_HW_BYTES_PER_SAMPLE_DATA);
        // DEBUG_PRINTF("  samples_count = %d\n", samples_count);
        //DEBUG_PRINTF("  A_HW_BYTES_PER_SAMPLE_DATA = %d\n", A_HW_BYTES_PER_SAMPLE_DATA);

        EnvironmentProcess(audioBuffer_, sz_, A_HW_BYTES_PER_SAMPLE_DATA, env_, det_);
        if (size_t num_words = DetectionProcess(audioBuffer_, sz_, A_HW_BYTES_PER_SAMPLE_DATA, &det_)) {
          //size_t samples_count = (sz_ / A_HW_BYTES_PER_SAMPLE_DATA);
          //DEBUG_PRINTF("  samples_count = %d\n", samples_count);

          // DEBUG_PRINTF("  num_words = %d\n", num_words);
          // DEBUG_PRINTF("  sz_ = %d : ", sz_);
          // DEBUG_PRINTF("  det_.fragment_byte_offset = %d\n", det_.fragment_byte_offset);
          if (det_.fragment_byte_offset) {
            sz_ -= det_.fragment_byte_offset; 
            std::memmove(audioBuffer_, reinterpret_cast<uint8_t *>(audioBuffer_) + det_.fragment_byte_offset, sz_);
          }
          if (sz_ >= hw_kws_duration_size_) {
            // DEBUG_PRINTF("[break_1]\n");
            break;
          }
          continue;
        }
        if (sz_ > hw_kws_half_duration_size_) {
          std::memmove(audioBuffer_, reinterpret_cast<uint8_t *>(audioBuffer_) + (sz_ - hw_kws_half_duration_size_), hw_kws_half_duration_size_);
          sz_ = hw_kws_half_duration_size_; 
        }
        return 0; //Ready. Waiting for your one word...
      } 
      while (true);
    } else {
      do {
        size_t kws_sz = 0;
        size_t read_size = std::min(hw_kws_dbl_duration_size_ - sz_, hw_kws_duration_size_);
        while((sz_ <= audio_buffer_size_ - hw_fragment_size_) && (kws_sz < read_size)) {
          size_t bytes = Read(reinterpret_cast<uint8_t *>(audioBuffer_) + sz_, hw_fragment_size_);
          kws_sz += bytes;
          sz_ += bytes;
        }
        if (det_.fragment_byte_size) {
          if (sz_ >= hw_kws_dbl_duration_size_)
            break;
          continue;
        }

        EnvironmentProcess(audioBuffer_, sz_, A_HW_BYTES_PER_SAMPLE_DATA, env_, det_);
        if (DetectionProcess(audioBuffer_, sz_, A_HW_BYTES_PER_SAMPLE_DATA, &det_)) {
          if (det_.fragment_byte_offset > hw_kws_duration_size_) {
            sz_ -= (det_.fragment_byte_offset - hw_kws_duration_size_);
            std::memmove(audioBuffer_, reinterpret_cast<uint8_t *>(audioBuffer_) + (det_.fragment_byte_offset - hw_kws_duration_size_), sz_); 
          }
          if (sz_ >= hw_kws_dbl_duration_size_)
            break;
          continue;
        }
        if (sz_ > hw_kws_duration_size_) {
          std::memmove(audioBuffer_, reinterpret_cast<uint8_t *>(audioBuffer_) + (sz_ - hw_kws_duration_size_), hw_kws_duration_size_);
          sz_ = hw_kws_duration_size_; 
        }
        return 0;
      } 
      while (true);
    }
    fz_ = sz_;
    sz_ = 0;
    stage_ = 0;
    // DEBUG_PRINTF("  [return fz_;]\n");
    frag_time_ += esp_timer_get_time() - time_start_;
    return fz_;
  } // !pp

//   if (!dst && !flag) {
//     DEBUG_PRINTF("!dst && !flag\n");
//     if (btemplate) {
//       { // MFCC Template
//         ScopeTimer st("MFCC Template (Collect)");
// #ifdef A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE
//         size_t sz = Read(audioBuffer_, hw_kws_duration_size_);
//         DEBUG_PRINTF("sz1 = %d\n", sz);
//         sz = SampleProcess(audioBuffer_, sz);
//         DEBUG_PRINTF("sz2 = %d\n", sz);
// #else
//         size_t sz = kws_duration_size_;
// #endif // A_MFCC_ENVIRONMENT_PREPARED_TEMPLATE     
//         //pp->Apply(audioBuffer_, sz, dst);
//       }
//     } 
//     return 0; 
//   }

  if (!stage_ && flag) {
    // Process
    //DEBUG_PRINTF("Collect_8\n");
    // DEBUG_PRINTF("  [fragment_byte_offset = %d]\n", det_.fragment_byte_offset);
    //DEBUG_PRINTF("fragments.size() = %d\n", det_.fragments->size());
    // DEBUG_PRINTF("Signal processing...\n");
    DEBUG_SCOPE_TIMER("Process");
    fz_ = SampleProcess(audioBuffer_, fz_, &det_);
    // DEBUG_PRINTF("  {fragment_byte_offset = %d}\n", det_.fragment_byte_offset);
    // start = audioBuffer_[0];
    // end = audioBuffer_[fz_ / A_BYTES_PER_SAMPLE_DATA];
    size_t samples_count = (fz_ / A_BYTES_PER_SAMPLE_DATA);
#ifdef REMOVE_SILENCE_COEFF
    size_t SignalLen = samples_count * REMOVE_SILENCE_COEFF; // removing extra silence at the end of the recording
#else
    size_t SignalLen = samples_count; // removing extra silence at the end of the recording
#endif

    DEBUG_SCOPE_TIMER("MFCC");
    float WinLen = KWS_WIN_MS / 1000.0;
    float WinStep = KWS_STRIDE_MS /1000.0;
    int frame_len = (int)std::round(WinLen * A_SAMPLE_RATE);
    int frame_step = (int)std::round(WinStep * A_SAMPLE_RATE);

    int n_frames;
    if (SignalLen > frame_len)
      n_frames = 1 + (int)std::ceil((SignalLen - frame_len) /
                 (float)frame_step);
    else 
      n_frames = 1;

    float** MFCC_data = reinterpret_cast<float**>(dst);
    (*MFCC_data) = new float[n_frames * KWS_NUM_MFCC];
    // DEBUG_PRINTF("\n          &(*MFCC_data) = %p\n", &(*MFCC_data));
    // DEBUG_PRINTF("n_frames = %d\n", n_frames);
    size_t ret = pp->Apply(audioBuffer_, SignalLen, *MFCC_data, n_frames);
    frag_time_ += esp_timer_get_time() - time_start_;
    return ret;
  }

  return 0;
}

// Change envieronment
void MicFragmentReader::EnvironmentArrayRightshift() {
  for (int i = env_.max_array_size; i > 0; i--)
	env_.max_abs_array[i] = env_.max_abs_array[i - 1];
}

// Analysis of the environment ('silence') signal level 
void MicFragmentReader::EnvironmentProcess(void* buf, const size_t bytes, const size_t bytes_width, Environment& env, Detection& det) {
  env.max_abs = 0;

#ifdef USE_AVERAGE_SIG
  AverageSigProcess(buf, bytes, A_HW_BYTES_PER_SAMPLE_DATA);
#endif

  const size_t samples = bytes / bytes_width;
  if (samples && buf && bytes && bytes_width) {
    proc_t sum = proc_t(0);
    uint8_t* s = reinterpret_cast<uint8_t*>(buf);
    int32_t val;
    for (size_t z = 0; z < samples; z++) {
      val = (s[bytes_width - 1] & 0x80) ? -1 : 0;
      for(int b = bytes_width - 1; b >= 0; b-- )
        val = (val << 8) | (s[b] & 0xFF);
      s += bytes_width;

      val = std::abs(val);
      sum += static_cast<proc_t>(val);
      if (env.max_abs < val)
        env.max_abs = val;
    }
  }

  // shift window left
  if ((env.max_abs_array[env.max_array_size - 1] < env.max_abs_array[env.max_array_size - 2] * 2) || (env.max_abs_array[env.max_array_size - 1] < env.max_abs * 2))
    for (int i = 0; i < env.max_array_size - 1; i++)
      env.max_abs_array[i] = env.max_abs_array[i + 1];
  env.max_abs_array[env.max_array_size - 1] = env.max_abs;

  // calc max abs
  int32_t min = 0, max = 0, sum = 0;
  for (int i = 0; i < env.max_array_size; i++) {
    min = std::min(min, env.max_abs_array[i]);
    max = std::max(max, env.max_abs_array[i]);
    sum += env.max_abs_array[i];
  }

  // update environment
  max = std::max(max, (long int)150);
  int32_t min_max_avg = (min + max) / 2 * 1.2;
  int32_t sum_avg = sum / env.max_array_size;
  if (min_max_avg <= sum_avg && det.environment != max) {
	DEBUG_PRINTF("new environment: %ld (%s)\n", max, (max < 400) ? "good" : (max < 800) ? "normal" : (max < 4000) ? "so-so" : "noisily");
	det.environment = max;
  }
}

/*! 
 * \brief Convert a stream and detect a useful stream fragment
 *
 * \param buf[in] buffer of analyzed stream data
 * \param bytes[in] buffer size
 * \param det[in,out] pointer to a structure with fragment detection parameters and detection results
 * \return conversion stream data buffer size
 */
size_t MicFragmentReader::SampleProcess(void* buf, size_t bytes, Detection* det) {
  // DEBUG_PRINTF("\n-------------> SampleProcess() ------------->\n");
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

  if ((A_BYTES_PER_SAMPLE_DATA == A_HW_BYTES_PER_SAMPLE_DATA) && (SP_VOLUME_SHT == 0) && !det)
    return bytes;

  uint8_t* s = reinterpret_cast<uint8_t*>(buf);
  uint8_t* d = reinterpret_cast<uint8_t*>(buf);
  const size_t samples = bytes / A_HW_BYTES_PER_SAMPLE_DATA;

  if (!det) {
    // No fragment detection
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
  else {
    // Requirement to detect fragments
    FragmentDetector detector(*det);
    detector.Init(samples, A_BYTES_PER_SAMPLE_DATA);

    int32_t val;
    int sat;
    for (size_t z = 0; z < samples; z++) {
      // Collect the value of the sample reference
      sat = 0;
      if (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & 0x80) {
        val = -1;
        if ((s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & SP_VOLUME_MSK) != SP_VOLUME_MSK) {
          sat = 1;
          for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++)
            d[i] = 0;
          d[A_BYTES_PER_SAMPLE_DATA - 1] |= 0x80;
        }
      }
      else {
        val = 0;
        if (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & SP_VOLUME_MSK) {
          sat = 1;
          for (int i = 0; i < A_BYTES_PER_SAMPLE_DATA; i++)
            d[i] = 0xFF;
          d[A_BYTES_PER_SAMPLE_DATA - 1] &= 0x7F;
        }
      }
      
      val = (val << 8) | (s[A_HW_BYTES_PER_SAMPLE_DATA - 1] & 0xFF);
#if A_HW_BYTES_PER_SAMPLE_DATA == 2
      val = (val << 8) | (s[A_HW_BYTES_PER_SAMPLE_DATA - 2] & 0xFF);
#elif A_HW_BYTES_PER_SAMPLE_DATA == 3
      val = (((val << 8) | (s[A_HW_BYTES_PER_SAMPLE_DATA - 2] & 0xFF)) << 8)
        | (s[A_HW_BYTES_PER_SAMPLE_DATA - 3] & 0xFF);
#elif A_HW_BYTES_PER_SAMPLE_DATA == 4
      val = (((((val << 8) | (s[A_HW_BYTES_PER_SAMPLE_DATA - 2] & 0xFF)) << 8)
        | (s[A_HW_BYTES_PER_SAMPLE_DATA - 3] & 0xFF)) << 8)
        | (s[A_HW_BYTES_PER_SAMPLE_DATA - 4] & 0xFF);
#endif
      detector.Detect(val);
      if (!sat) {
        // Level increase
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
    } // for z
    return samples * A_BYTES_PER_SAMPLE_DATA;
  }
}

/*! 
 * \brief Shift a stream fragment to zero
 *
 * \param buf[in] buffer of analyzed stream data
 * \param bytes[in] buffer size
 * \param bytes_width[in] sample size
 */
void MicFragmentReader::AverageSigProcess(void* buf, const size_t bytes, const size_t bytes_width) {
	const size_t samples = bytes / bytes_width;
	audio_t* raw_signal = reinterpret_cast<audio_t*>(buf);
	int64_t summ_samples = 0;
	float average_samples = 0;
	for (size_t i = 0; i < samples; i++)
	  summ_samples += raw_signal[i];
	average_samples = summ_samples / samples;

	for (size_t i = 0; i < samples; i++)
	  raw_signal[i] -= average_samples;
}

/*! 
 * \brief Detect a useful stream fragment
 *
 * \param buf[in] buffer of analyzed stream data
 * \param bytes[in] buffer size
 * \param bytes_width[in] sample size
 * \param det[in,out] pointer to a structure with fragment detection parameters and detection results
 * \return the number of detected words
 */
size_t MicFragmentReader::DetectionProcess(void* buf, const size_t bytes, const size_t bytes_width, Detection* det) {
  if (!buf || !bytes || !bytes_width || !det)
    return 0;

  const size_t samples = bytes / bytes_width;
  uint8_t* s = reinterpret_cast<uint8_t*>(buf);
  FragmentDetector detector(*det);
  detector.Init(samples, bytes_width);

  int32_t val;
  for (size_t z = 0; z < samples; z++) {
    // Collect the value of the sample reference
    val = (s[bytes_width - 1] & 0x80) ? -1 : 0;
    for(int b = bytes_width - 1; b >= 0; b-- )
      val = (val << 8) | (s[b] & 0xFF);
    s += bytes_width;
    detector.Detect(val);
  } //for z

  return (det->fragments) ? det->fragments->size() : (det->fragment_byte_size) ? 1 : 0; 
}

