#pragma once
#include "BaseGrc.hpp"

#define SIGNAL_SAMPLES_NUM 2000
#define SIGNAL_COMPS_NUM 6
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 5

/*!
 * \brief App to recognize gestures.
 */
class GestureGrc : public BaseGrc {
public:
  /*!
  * \brief Hyper parameters to GRC AI SW.
  */
  static constexpr HP hp = {
    .PredictSignal = false,
    .SeparateInaccuracies = false,
    .Noise = 0.00143976,
    .InputScaling = 0.524927,
    .FeedbackScaling = 0.000898804,
    .ThresholdFactor = 1.05627
  };
  /*! \brief Constructor. */
  GestureGrc() : BaseGrc("Gesture") {}
  /*!
   * \brief Train Grc on signal.
   * \param signal The input signal that will be sent to Grc.
   * \param category Overwrite specific category in Grc.
   * \return Trained category.
   */
  int train(Matrix &signal, int category = -1) override final;
  /*!
   * \brief Inference on signal.
   * \param signal The input signal that will be sent to Grc.
   * \return Inferenced category.
   */
  int inference(Matrix &signal) override final;
  /*!
    * \brief Gesture app specific preprocessing.
    * \param signal Input signal.
    */
  void preprocess(Matrix &signal) const;
};
/*!
  * \brief Determine the beginning of the signal.
  * \param src Input signal.
  * \param percent Threshold.
  * \return Index (-1 if not found).
  */
int getSignalBeg(const Matrix &src, unsigned percent);