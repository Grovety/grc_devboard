#pragma once

/*!
 * \brief Interface to buttons.
 */
class IButton {
public:
  /*!
   * \brief Constructor.
   * \param pin GPIO pin.
   */
  IButton(int pin) : pin_(pin) {}
  /*!
   * \brief Get button state.
   * \return Button state.
   */
  virtual bool isPressed() const = 0;

protected:
  /*! \brief GPIO pin. */
  int pin_;
};
