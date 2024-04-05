#ifndef _MIC_READER_H_
#define _MIC_READER_H_

/*!
 * \brief Initialize microphone data reader and processor.
 * \return Result.
 */
bool mic_reader_init();
/*!
 * \brief Release microphone data reader and processor.
 */
void mic_reader_release();
/*!
 * \brief Get current microphone environment.
 */
size_t mic_reader_get_env();

#endif // _MIC_READER_H_
