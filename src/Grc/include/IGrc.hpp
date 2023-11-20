#pragma once
#include "custom_types.hpp"

#include <vector>

/*!
 * \brief Interface to communicate with Grc.
 */
class IGrc {
protected:
  /*!
   * \brief Initialize Grc device and GRS AI SW.
   * \param hp Hyper parameters to GRS AI SW.
   * \return Error code.
   */
  // TODO use sdk hp
  int initReq(const HP &hp) const;
  /*!
   * \brief Clear GRS AI SW state.
   * \return Error code.
   */
  int clearReq() const;
  /*!
   * \brief Train GRS AI SW on raw data.
   * \param len Train data len.
   * \param vals Pointer to train data.
   * \param category Overwrite specific category in GRS AI SW.
   * \return Trained category.
   */
  int trainReq(unsigned len, const float *vals, int category) const;
  /*!
   * \brief Inference on raw data.
   * \param len Inference data len.
   * \param vals Pointer to inference data.
   * \param category Hint category.
   * \return Inferenced category.
   */
  int inferenceReq(unsigned len, const float *vals, int category = -1) const;
  /*!
   * \brief Get the number of trained categories.
   * \return Number of trained categories.
   */
  unsigned getQtyReq() const;
  /*!
   * \brief Retrieve train metadata from GRS .
   * \param data Where to save.
   * \return Number of trained categories.
   */
  unsigned saveTrainDataReq(std::vector<RT> &data) const;
  /*!
   * \brief Load train metadata.
   * \param qty Number of trained categories.
   * \param len Buffer size.
   * \param val Pointer to data.
   * \return Whether the data was sent successfully to Grc.
   */
  bool loadTrainDataReq(unsigned qty, unsigned len, const RT *vals) const;
};
