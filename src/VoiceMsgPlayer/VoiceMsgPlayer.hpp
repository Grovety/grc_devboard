#pragma once
#include "WavPlayer.hpp"

using VoiceMsgId = size_t;

/*!
 * \brief Play wav from table.
 * \param id Sample id.
 */
void VoiceMsgPlay(VoiceMsgId id);
/*!
 * \brief Stop playback.
 */
void VoiceMsgStop();
/*!
 * \brief Blocking wait playback stop.
 * \param xTicks Ticks to wait.
 * \return Timeout result.
 */
bool VoiceMsgWaitStop(size_t xTicks);