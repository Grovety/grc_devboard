#include "RhythmGrc.hpp"
#include "preprocessing.hpp"

int RhythmGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  if (!preprocessRhythm(input, SIGNAL_PERIOD_MS)) {
    LOGE(name_, "Error preprocessing signal");
  }
  const auto vec = saveToVect(input);

  return trainReq(vec.size(), vec.data(), category);
}

int RhythmGrc::inference(const MatrixDyn &signal) {
  auto input = signal;
  if (!preprocessRhythm(input, SIGNAL_PERIOD_MS)) {
    LOGE(name_, "Error preprocessing signal");
  }
  const auto vec = saveToVect(input);

  const auto category = inferenceReq(vec.size(), vec.data());
  return category;
}