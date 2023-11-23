#pragma once
#include "BaseGrc.hpp"

#define SIGNAL_SAMPLES_NUM 1500
#define SIGNAL_COMPS_NUM 3
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 5

/*!
 * \brief App to recognize rhythms.
 */
class RhythmGrc : public BaseGrc {
public:
  /*!
  * \brief Hyper parameters to GRC AI SW.
  */
  static constexpr HP hp = {
    .PredictSignal = true,
    .SeparateInaccuracies = false,
    .InputComponents = 1,
    .OutputComponents = 1,
    .Neurons = 18,
    .SpectralRadius = 0.444104,
    .Sparsity = 0.631049,
    .Noise = 0.00100195,
    .InputScaling = 0.566296,
    .InputSparsity = 0.4302,
    .FeedbackScaling = 0.00168945,
    .FeedbackSparsity = 0.417676,
    .ThresholdFactor = 0.86792
  };
  /*! \brief Constructor. */
  RhythmGrc() : BaseGrc("Rhythm") {}
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
/*!
  * \brief Rhythm app specific preprocessing.
  * \param signal Input signal.
  * \param signal_period_ms Sensor polling frequency.
  * \return Result.
  */
bool preprocessRhythm(MatrixDyn &signal, unsigned signal_period_ms);