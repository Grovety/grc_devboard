#pragma once
#include <stdarg.h>

/*!
 * \brief Interface to displays.
 */
class IDisplay {
public:
  virtual ~IDisplay() = default;
  /*!
   * \brief Print current status message.
   * \param msg Message string.
   */
  virtual void print_status(const char *msg) = 0;
  /*!
   * \brief Print current context message.
   * \param fmt Formatted message string.
   */
  virtual void print_header(const char *fmt, ...) = 0;
  /*!
   * \brief Print string.
   * \param fmt Formatted message string.
   */
  virtual void print_string(const char *fmt, ...) = 0;
  /*!
   * \brief Send buffer to display.
   */
  virtual void send() = 0;
  /*!
   * \brief Clear display buffer.
   */
  virtual void clear() = 0;
};
