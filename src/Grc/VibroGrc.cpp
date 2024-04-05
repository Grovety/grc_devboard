#include "VibroGrc.hpp"

static float calcVectLen(const Matrix &vec) {
  auto sumSq = [](Matrix &mat, unsigned row, unsigned col, void *parm) {
    const float val = mat(row, col);
    *(float *)parm += val * val;
    return WC_NEXT;
  };
  float sum = 0;
  walk(const_cast<Matrix &>(vec), sumSq, &sum);
  return sqrt(sum);
}

static void shift(Matrix &signal, SignalStat &tstat) {
  auto shift = [](Matrix &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) -= (*(Matrix *)parm)(row, 0);
    return WC_NEXT;
  };
  walk(signal, shift, &tstat.Mean);
}

static void scale(Matrix &signal, SignalStat &tstat) {
  auto scale = [](Matrix &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) /= 2 * *(float *)parm;
    return WC_NEXT;
  };
  float len = calcVectLen(tstat.StDev);
  walk(signal, scale, &len);
}

static void adjustSignal(Matrix &signal, SignalStat &tstat) {
  shift(signal, tstat);
  scale(signal, tstat);
}

static bool checkThreshold(const Matrix &val, const Matrix &eta,
                           const Matrix &stdv, float factor) {
  auto check = [](Matrix &mat, unsigned row, unsigned col, void *parm) {
    return mat(row, col) >= 0 ? WC_NEXT : WC_STOP;
  };

  Matrix diff = val - (eta - stdv * (factor / 2));
  if (!walk(diff, check, NULL))
    return false;

  diff = (eta + stdv * (factor / 2)) - val;
  if (!walk(diff, check, NULL))
    return false;

  return true;
}

int VibroGrc::train(Matrix &signal, int category) {
  if (category <= -1 || category >= train_stats_.size()) {
    train_stats_.push_back({});
  }
  auto &tstat = train_stats_.back();
  tstat.calcSignal(signal, hp.SeparateInaccuracies);
  adjustSignal(signal, tstat);
  const auto vec = saveToVect(signal);
  return grc_.train(vec.size(), vec.data(), category);
}

int VibroGrc::inference(Matrix &signal) {
  // use train stat info for each category to preprocess the signal
  for (int i = 0; i < train_stats_.size(); ++i) {
    auto input = signal;

    InferStat istat;
    preprocess(input, istat, train_stats_[i]);
    if (istat.fail) {
      continue;
    }
    const auto vec = saveToVect(input);

    const auto category = grc_.inference(vec.size(), vec.data(), i);
    if (category > -1) {
      return category;
    }
  }
  return -1;
}

unsigned VibroGrc::save(std::vector<float> &data) {
  const auto qty = BaseGrc::save(data);

  for (const auto &tstat : train_stats_) {
    for (unsigned i = 0; i < tstat.Mean.numRows(); i++)
      for (unsigned j = 0; j < tstat.Mean.numCols(); j++)
        data.push_back(tstat.Mean(i, j));
    for (unsigned i = 0; i < tstat.StDev.numRows(); i++)
      for (unsigned j = 0; j < tstat.StDev.numCols(); j++)
        data.push_back(tstat.StDev(i, j));
  }

  return qty;
}

bool VibroGrc::load(unsigned qty, const std::vector<float> &data) {
  const auto _data = loadSignalStat(qty, data);
  if (BaseGrc::load(qty, _data)) {
    return true;
  }
  return false;
}

std::vector<float> VibroGrc::loadSignalStat(unsigned qty,
                                            const std::vector<float> &data) {
  LOGD(name_, "load train stat");

  train_stats_.clear();
  const unsigned rows = hp.SeparateInaccuracies ? SIGNAL_COMPS_NUM : 1;
  const unsigned cols = 1;
  const unsigned fields_num = 2;
  const auto matrix_sz = rows * cols;
  const auto total_sz = qty * matrix_sz * fields_num;
  const auto *wgt_beg = data.data();
  if (total_sz < data.size()) {
    // skip weights
    const auto *wgt_end = wgt_beg + data.size() - total_sz;
    for (unsigned i = 0; i < qty; i++) {
      train_stats_.push_back({});
      auto &tstat = train_stats_.back();
      const auto offset = i * matrix_sz * fields_num;
      tstat.Mean = Matrix((Matrix::ElmType *)wgt_end + offset, rows, cols, true,
                          false, false);
      tstat.StDev = Matrix((Matrix::ElmType *)wgt_end + offset + matrix_sz,
                           rows, cols, true, false, false);
    }
    return std::vector<float>(wgt_beg, wgt_end);
  }
  return data;
}

int VibroGrc::clear() {
  train_stats_.clear();
  return BaseGrc::clear();
}

void VibroGrc::preprocess(Matrix &signal, InferStat &istat,
                          SignalStat &tstat) const {
  LOGD(name_, __FUNCTION__);

  istat.calcSignal(signal, hp.SeparateInaccuracies);

  rotate(signal, istat, tstat);

  istat.fail = 0;
  if (!checkThreshold(istat.Mean, tstat.Mean, tstat.StDev,
                      float(hp.ThresholdFactor))) {
    istat.mean_fail = true;
  }
  if (!checkThreshold(istat.StDev, tstat.StDev, tstat.StDev,
                      float(hp.ThresholdFactor))) {
    istat.stdev_fail = true;
  }

  adjustSignal(signal, tstat);
}

void VibroGrc::rotate(Matrix &signal, InferStat &istat,
                      SignalStat &tstat) const {
  const float coef = 1;
  Matrix rot = tstat.Mean * coef - istat.Mean;
  rot = rot.repeat(1, signal.numCols());
  signal += rot;

  istat.calcSignal(signal, hp.SeparateInaccuracies);
}
