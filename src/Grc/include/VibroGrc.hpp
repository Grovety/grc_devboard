#pragma once
#include "BaseGrc.hpp"
#include "Statistics.hpp"

#define SIGNAL_SAMPLES_NUM 240
#define SIGNAL_COMPS_NUM 3
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 5

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
    .InputComponents = 3,
    .OutputComponents = 3,
    .Neurons = 19,
    .SpectralRadius = 0.45813,
    .Sparsity = 0.255005,
    .Noise = 0.00111267,
    .InputScaling = 0.357373,
    .InputSparsity = 0.507581,
    .FeedbackScaling = 0.000584534,
    .FeedbackSparsity = 0.773157,
    .ThresholdFactor = 0.969788
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
  int train(const MatrixDyn &signal, int category = -1) override final;
  /*!
   * \brief Inference on signal.
   * \param signal The input signal that will be sent to Grc.
   * \return Inferenced category.
   */
  int inference(const MatrixDyn &signal) override final;
  /*!
   * \brief Save train metadata to buffer.
   * \param data Buffer.
   * \return Number of saved categories.
   */
  unsigned save(std::vector<RT> &data) override final;
  /*!
   * \brief Load train metadata to Grc and extract train stat info.
   * \param qty Number of categories.
   * \param data Buffer.
   * \return Whether the data was loaded successfully to Grc.
   */
  bool load(unsigned qty, const std::vector<RT> &data) override final;
  /*!
   * \brief Load train stat info to VibroGrc.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   * \return Modified train metadata.
   */
  std::vector<RT> loadSignalStat(unsigned qty, const std::vector<RT> &data);

private:
  /*!
    * \brief Vibro app specific preprocessing.
    * \param signal Input signal.
    * \param istat Infer signal statistics.
    * \param tstat Train signal statistics.
    */
  void preprocess(MatrixDyn &signal, InferStat& istat, SignalStat &tstat) const;
  /*!
    * \brief Apply rotation to signal axes.
    * \param signal Input signal.
    * \param istat Infer signal statistics.
    * \param tstat Train signal statistics.
    */
  void rotate(MatrixDyn &signal, InferStat &istat, SignalStat &tstat) const;
  /*! \brief Train stat info for each category. */
  std::vector<SignalStat> train_stats_;
};
