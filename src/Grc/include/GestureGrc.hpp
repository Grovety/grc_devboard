#pragma once
#include "BaseGrc.hpp"

#define SIGNAL_SAMPLES_NUM 1500
#define SIGNAL_COMPS_NUM 6
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 5

/*!
 * \brief App to recognize gestures.
 */
class GestureGrc : public BaseGrc {
public:
  GestureGrc() : BaseGrc("Gesture") {}
  /*!
   * \brief Train Grc on signal.
   * \param signal The input signal that will be sent to Grc.
   * \param category Overwrite specific category in Grc.
   * \return Trained category.
   */
  int train(const MatrixDyn &signal, int category = -1) override final;
  /*!
   * \brief Inference on signal.
   * \param signal The input signal that will be sent to Grc.
   * \return Inferenced category.
   */
  int inference(const MatrixDyn &signal) override final;
};
