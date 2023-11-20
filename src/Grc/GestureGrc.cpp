#include "GestureGrc.hpp"
#include "preprocessing.hpp"

int GestureGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  smooth(input, signal, 10, hyper_params_.m_InputComponents, 0);
  auto temp = input;
  concat(input, temp);
  const auto vec = saveToVect(input);

  return trainReq(vec.size(), vec.data(), category);
}

int GestureGrc::inference(const MatrixDyn &signal) {
  auto input = signal;
  smooth(input, signal, 10, hyper_params_.m_InputComponents, 0);
  auto temp = input;
  concat(input, temp);
  const auto vec = saveToVect(input);

  const auto category = inferenceReq(vec.size(), vec.data());
  return category;
}