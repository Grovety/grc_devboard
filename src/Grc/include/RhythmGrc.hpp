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
    .Noise = 0.00100195,
    .InputScaling = 0.566296,
    .FeedbackScaling = 0.00168945,
    .ThresholdFactor = 0.86792
  };
  /*! \brief Constructor. */
  RhythmGrc() : BaseGrc("Rhythm") {}
  /*!
   * \brief Clear Grc state.
   * \return Error code.
   */
  int clear() override final;
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
   * \brief Save train metadata to buffer.
   * \param data Buffer.
   * \return Number of saved categories.
   */
  unsigned save(std::vector<float> &data) override final;
  /*!
   * \brief Load train metadata to Grc and extract train stat info.
   * \param qty Number of categories.
   * \param data Buffer.
   * \return Whether the data was loaded successfully to Grc.
   */
  bool load(unsigned qty, const std::vector<float> &data) override final;

private:
  /*! \brief Number of peaks for each category. */
  std::vector<size_t> peaks_num_;
};
/*!
  * \brief Rhythm app specific preprocessing.
  * \param signal Input signal.
  * \param signal_period_ms Sensor polling frequency.
  * \return Result.
  */
bool preprocessRhythm(Matrix &signal, float signal_period_ms);