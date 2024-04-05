#include "RhythmGrc.hpp"

int RhythmGrc::train(Matrix &signal, int category) {
  const unsigned peaks_num = (signal.getCols() / 16 - 2) / 2 + 1;
  peaks_num_.push_back(peaks_num);
  const auto vec = saveToVect(signal);
  return grc_.train(vec.size(), vec.data(), category);
}

int RhythmGrc::inference(Matrix &signal) {
  const auto vec = saveToVect(signal);
  const unsigned peaks_num = (signal.getCols() / 16 - 2) / 2 + 1;

  for (int i = 0; i < peaks_num_.size(); ++i) {
    if (peaks_num_[i] == peaks_num) {
      const auto category = grc_.inference(vec.size(), vec.data(), i);
      if (category > -1) {
        return category;
      }
    }
  }
  return -1;
}

unsigned RhythmGrc::save(std::vector<float> &data) {
  const auto qty = BaseGrc::save(data);

  for (const auto &elem : peaks_num_) {
    data.push_back(elem);
  }

  return qty;
}

bool RhythmGrc::load(unsigned qty, const std::vector<float> &data) {
  peaks_num_.resize(qty);
  const auto sz = data.size();
  for (size_t i = 0; i < qty; i++) {
    peaks_num_[i] = data[sz - (qty - i)];
  }
  const auto _data = std::vector<float>(data.begin(), data.end() - qty);
  if (BaseGrc::load(qty, _data)) {
    return true;
  }
  return false;
}

int RhythmGrc::clear() {
  peaks_num_.clear();
  return BaseGrc::clear();
}

static bool dist(const Matrix &mat, float marg, unsigned col, unsigned &beg,
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

static bool scan(Matrix &dst, const Matrix &src) {
  dst.resize(1, 0, false);
 
  Matrix mean;
  src.calcStat(mean, StatMMAS<Matrix::ElmType>::Avg, false, false);
  float marg = 3 * (float)mean(0, 0);
 
  unsigned prev, end;
  if (!dist(src, marg, 0, prev, end))
    return false;
 
  for (unsigned col = end, beg; dist(src, marg, col, beg, end); prev = beg, col = end) {
    Matrix tmp(1, 2);
    tmp.setVal((float)(beg - prev));
    dst.concatCols(tmp);
  }
 
  dst.concatCols(Matrix(1, 3, true));
 
  return true;
}

static void collect(Matrix &dst, const Matrix &src) {
  Matrix mean;
  src.calcStat(mean, StatMMAS<Matrix::ElmType>::Avg, false, true);
 
  dst.resize(1, src.getCols(), false);
  for (unsigned col = 0; col < src.getCols(); ++col) {
    dst(0, col) = 0;
    for (unsigned row = 0; row < src.getRows(); ++row)
      dst(0, col) += ::fabs(src(row, col) - mean(row, 0));
  }
}

static void contrast(Matrix &dst, const Matrix &src, unsigned avg_n,
                     unsigned comps, unsigned beg) {
  unsigned cols = src.numCols() / avg_n;
  dst = Matrix(comps, cols, true);
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

bool preprocessRhythm(Matrix &signal, float signal_period_ms) {
  const unsigned max_signal_points = 50;
  const float max_dur_ms = 500;
  const float smooth_factor = max_dur_ms / max_signal_points; // 10
  const float peak_factor =
      max_dur_ms / (signal_period_ms * smooth_factor); // 25
  const unsigned repl_qty = 4;

  {
    Matrix tmp;
    collect(tmp, signal);
    signal = tmp;
  }
  {
    Matrix tmp;
    contrast(tmp, signal, (unsigned)smooth_factor, 1, 0);
    signal = tmp;
  }
  {
    Matrix tmp;
    if (!scan(tmp, signal))
      return false;
    signal = tmp;
  }
  signal *= (float)(1. / peak_factor);
  signal = signal.repeat(1, 1 << repl_qty);

  return true;
}
