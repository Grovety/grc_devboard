#pragma once
#include "BaseGrc.hpp"

#define SIGNAL_SAMPLES_NUM 240
#define SIGNAL_COMPS_NUM 3
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 4

/*!
 * \brief App to recognize vibration type.
 */
class VibroGrc : public BaseGrc {
public:
  /*!
  * \brief Hyper parameters to GRC AI SW.
  */
  static constexpr HP hp = {
    .PredictSignal = false,
    .SeparateInaccuracies = true,
    .Noise = 0.00155414,
    .InputScaling = 0.365772,
    .FeedbackScaling = 0.00120148,
    .ThresholdFactor = 0.678436
  };
  /*! \brief Constructor. */
  VibroGrc() : BaseGrc("Vibro") {}
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
  /*!
   * \brief Load train stat info to VibroGrc.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   * \return Modified train metadata.
   */
  std::vector<float> loadSignalStat(unsigned qty, const std::vector<float> &data);

private:
  /*!
    * \brief Vibro app specific preprocessing.
    * \param signal Input signal.
    * \param istat Infer signal statistics.
    * \param tstat Train signal statistics.
    */
  void preprocess(Matrix &signal, InferStat& istat, SignalStat &tstat) const;
  /*!
    * \brief Apply rotation to signal axes.
    * \param signal Input signal.
    * \param istat Infer signal statistics.
    * \param tstat Train signal statistics.
    */
  void rotate(Matrix &signal, InferStat &istat, SignalStat &tstat) const;
  /*! \brief Train stat info for each category. */
  std::vector<SignalStat> train_stats_;
};
