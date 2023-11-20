#pragma once
#include "BaseGrc.hpp"

#define SIGNAL_SAMPLES_NUM 240
#define SIGNAL_COMPS_NUM 3
#define SIGNAL_PERIOD_MS 2
#define MAX_TRAINABLE_CATEGORIES 5

/*!
 * \brief App to recognize vibration type.
 */
class VibroGrc : public BaseGrc {
public:
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
  std::vector<RT> loadTrainStat(unsigned qty, const std::vector<RT> &data);

private:
  /*! \brief Train stat info for each category. */
  std::vector<TrainStat> train_stats_;
};
