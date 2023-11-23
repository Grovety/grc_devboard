#pragma once
#include "grc/Grc.hpp"
#include "Common.hpp"

/*!
 * \brief Interface to Grc applications.
 */
class BaseGrc {
public:
  /*!
   * \brief The constructor.
   * \param name Application name.
   */
  BaseGrc(const char *name);
  /*! \brief The virtual destructor. */
  virtual ~BaseGrc();
  /*!
   * \brief Grc initialization.
   * \param hp Hyper parameters to Grc.
   * \return Error code.
   */
  virtual int init(HP hp);
  /*!
   * \brief Clear Grc state.
   * \return Error code.
   */
  virtual int clear();
  /*!
   * \brief Train Grc on signal.
   * \param signal The input signal that will be sent to Grc.
   * \param category Overwrite specific category in Grc.
   * \return Trained category.
   */
  virtual int train(const MatrixDyn &signal, int category = -1) = 0;
  /*!
   * \brief Inference on signal.
   * \param signal The input signal that will be sent to Grc.
   * \return Inferenced category.
   */
  virtual int inference(const MatrixDyn &signal) = 0;
  /*!
   * \brief Save train metadata to buffer.
   * \param data Buffer.
   * \return Number of saved categories.
   */
  virtual unsigned save(std::vector<RT> &data);
  /*!
   * \brief Load train metadata to Grc.
   * \param qty Number of categories.
   * \param data Buffer.
   * \return Whether the data was loaded successfully to Grc.
   */
  virtual bool load(unsigned qty, const std::vector<RT> &data);
  /*! \brief Get number of trained categories. */
  unsigned getQty() const;
  /*! \brief Get app name. */
  const char *getName() const;

protected:
  /*! \brief Grc device. */
  Grc grc_;
  /*! \brief App name. */
  const char *name_;
};
