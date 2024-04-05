#include "VoiceMsgPlayer.hpp"
#include "I2sTx.hpp"
#include "Types.hpp"

#include "string.h"

static const char *TAG = "VoiceMsgPlayer";

extern unsigned int g_voice_msgs_num;
extern const unsigned char *g_voice_msgs[];

void VoiceMsgPlay(VoiceMsgId id) {
  if (id == 0 || id > g_voice_msgs_num)
    return;
  VoiceMsgId array_idx = id - 1;
  wav_header_t header;
  memcpy(&header, g_voice_msgs[array_idx], sizeof(wav_header_t));
  Sample_t wav = {.data = g_voice_msgs[array_idx] + sizeof(wav_header_t),
                    .bytes = header.subchunk2Size};

  LOGD(TAG, "play msg: %d", id);
  for (size_t i = 0; i < wav.bytes / I2S_TX_AUDIO_BUFFER; i++) {
    const uint8_t *ptr = static_cast<const uint8_t*>(wav.data) + i * I2S_TX_AUDIO_BUFFER;
    Sample_t sample = {.data = ptr, .bytes = I2S_TX_AUDIO_BUFFER};
    xQueueSend(xWavPlayerQueue, &sample, portMAX_DELAY);
    LOGD(TAG, "send[%d]=%d", i, I2S_TX_AUDIO_BUFFER);
  }
  const size_t rem = wav.bytes % I2S_TX_AUDIO_BUFFER;
  if (rem != 0) {
    const uint8_t *ptr = static_cast<const uint8_t*>(wav.data) + wav.bytes - rem;
    Sample_t sample = {.data = ptr, .bytes = rem};
    xQueueSend(xWavPlayerQueue, &sample, portMAX_DELAY);
    LOGD(TAG, "send[-]=%d", rem);
  }
}

void VoiceMsgStop() {
  if (size_t samples_num = uxQueueMessagesWaiting(xWavPlayerQueue)) {
    LOGD(TAG, "dropped samples=%d", samples_num);
    xQueueReset(xWavPlayerQueue);
    VoiceMsgWaitStop(portMAX_DELAY);
  }
}

bool VoiceMsgWaitStop(size_t xTicks) {
  const auto xBits = xEventGroupWaitBits(xWavPlayerEventGroup, WAV_PLAYER_STOP_MSK, pdFALSE, pdFALSE, xTicks);
  return xBits & WAV_PLAYER_STOP_MSK;
}