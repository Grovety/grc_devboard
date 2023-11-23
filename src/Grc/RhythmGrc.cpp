#include "RhythmGrc.hpp"
#include "Numbers.hpp"
#include "Statistics.hpp"

int RhythmGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  if (!preprocessRhythm(input, SIGNAL_PERIOD_MS)) {
    LOGE(name_, "Error preprocessing signal");
  }
  const auto vec = saveToVect(input);

  return grc_.train(vec.size(), vec.data(), category);
}

int RhythmGrc::inference(const MatrixDyn &signal) {
  auto input = signal;
  if (!preprocessRhythm(input, SIGNAL_PERIOD_MS)) {
    LOGE(name_, "Error preprocessing signal");
  }
  const auto vec = saveToVect(input);

  const auto category = grc_.inference(vec.size(), vec.data());
  return category;
}

static void resize(MatrixDyn &mat, unsigned rows, unsigned cols) {
  if (mat.numRows() && mat.numCols())
    mat.resizeSlow(rows, cols);
  else
    mat.resizeFast(rows, cols);
}

static void resizeAdd(MatrixDyn &mat, unsigned rows, unsigned cols) {
  resize(mat, mat.numRows() + rows, mat.numCols() + cols);
}

static bool dist(const MatrixDyn &mat, float marg, unsigned col, unsigned &beg,
                 unsigned &end) {
  for (; col < mat.numCols(); ++col) {
    if (mat(0, col) > marg) {
      beg = col;
      for (end = col; end < mat.numCols(); ++end)
        if (mat(0, end) < marg)
          break;
      return true;
    }
  }
  return false;
}

static bool scan(MatrixDyn &dst, const MatrixDyn &src) {
  dst.resizeFast(1, 0);

  float marg = 2 * (float)mean(src)(0, 0);

  unsigned prev, end;
  if (!dist(src, marg, 0, prev, end))
    return false;

  for (unsigned col = end, beg; dist(src, marg, col, beg, end);
       prev = beg, col = end) {
    resizeAdd(dst, 0, 2);
    dst(0, dst.numCols() - 1) = (RT)(beg - prev);
    dst(0, dst.numCols() - 2) = (RT)(beg - prev);
  }

  resizeAdd(dst, 0, 3);
  dst(0, dst.numCols() - 1) = 0;
  dst(0, dst.numCols() - 2) = 0;
  dst(0, dst.numCols() - 3) = 0;

  return true;
}

static void collect(MatrixDyn &dst, const MatrixDyn &src) {
  dst.resizeFast(1, src.numCols());

  for (unsigned col = 0; col < src.numCols(); ++col) {
    dst(0, col) = 0;
    for (unsigned row = 0; row < src.numRows(); ++row)
      dst(0, col) += ::fabs(src(row, col));
  }
}

static void contrast(MatrixDyn &dst, const MatrixDyn &src, unsigned avg_n,
                     unsigned comps, unsigned beg) {
  unsigned cols = src.numCols() / avg_n;
  dst = nc::zeros<RT>(comps, cols);
  auto *pstats = new StatMMAS<FloatNum<float>>[comps];
  for (unsigned dcol = 0, scol = 0; dcol < cols; ++dcol, scol += avg_n) {
    for (unsigned r = 0; r < comps; ++r)
      pstats[r].clear();

    for (unsigned c = 0; c < avg_n; ++c)
      for (unsigned r = 0; r < comps; ++r)
        pstats[r].put(src(beg + r, scol + c));

    for (unsigned r = 0; r < comps; ++r)
      dst(r, dcol) = pstats[r].getStDev();
  }
  delete[] pstats;
}

bool preprocessRhythm(MatrixDyn &signal, unsigned signal_period_ms) {
  const unsigned max_signal_points = 50;
  const float max_dur_ms = 500;
  const float smooth_factor = max_dur_ms / max_signal_points; // 10
  const float peak_factor =
      max_dur_ms / (signal_period_ms * smooth_factor); // 25
  const unsigned repl_qty = 4;

  {
    MatrixDyn tmp;
    collect(tmp, signal);
    signal = tmp;
  }
  {
    MatrixDyn tmp;
    contrast(tmp, signal, (unsigned)smooth_factor, 1, 0);
    signal = tmp;
  }
  {
    MatrixDyn tmp;
    if (!scan(tmp, signal))
      return false;
    signal = tmp;
  }
  signal /= (RT)peak_factor;
  for (unsigned cnt = 0; cnt < repl_qty; ++cnt) {
    MatrixDyn tmp = signal;
    concat(signal, tmp);
  }

  return true;
}
