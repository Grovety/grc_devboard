#pragma once

/*!
 * \brief Interface to sensors.
 */
class ISensor {
public:
  virtual ~ISensor() = default;
  /*!
   * \brief Init device.
   * \return Result.
   */
  virtual bool open() = 0;
  /*!
   * \brief Close device.
   * \return Result.
   */
  virtual bool close() = 0;
  /*!
   * \brief Read sensor data.
   * \param buf Pointer to allocated buffer.
   * \param len Buffer length.
   */
  virtual void getData(float *buf, unsigned len) = 0;
};
