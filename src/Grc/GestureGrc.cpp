#include "GestureGrc.hpp"

static void smooth(MatrixDyn &dst, const MatrixDyn &src, unsigned avg_n,
                   unsigned comps, unsigned beg) {
  unsigned cols = src.numCols() / avg_n;
  dst = nc::zeros<RT>(comps, cols);
  for (unsigned dcol = 0, scol = 0; dcol < cols; ++dcol) {
    for (unsigned c = 0; c < avg_n; ++c)
      for (unsigned r = 0; r < comps; ++r)
        dst(r, dcol) += src(beg + r, scol + c);

    scol += avg_n;

    for (unsigned r = 0; r < comps; ++r)
      dst(r, dcol) /= avg_n;
  }
}

int GestureGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  smooth(input, signal, 10, hp.InputComponents, 0);
  auto temp = input;
  concat(input, temp);
  const auto vec = saveToVect(input);

  return grc_.train(vec.size(), vec.data(), category);
}

int GestureGrc::inference(const MatrixDyn &signal) {
  auto input = signal;
  smooth(input, signal, 10, hp.InputComponents, 0);
  auto temp = input;
  concat(input, temp);
  const auto vec = saveToVect(input);

  const auto category = grc_.inference(vec.size(), vec.data());
  return category;
}
