#include "VibroGrc.hpp"

static RT calcVectLen(const MatrixDyn &vec) {
  auto sumSq = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    const RT val = mat(row, col);
    *(RT *)parm += val * val;
    return WC_NEXT;
  };
  RT sum = 0;
  walk(const_cast<MatrixDyn &>(vec), sumSq, &sum);
  return sqrt(sum);
}

static void shift(MatrixDyn &signal, SignalStat &tstat) {
  auto shift = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) -= (*(MatrixDyn *)parm)(row, 0);
    return WC_NEXT;
  };
  walk(signal, shift, &tstat.Mean);
}

static void scale(MatrixDyn &signal, SignalStat &tstat) {
  auto scale = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) /= 2 * *(RT *)parm;
    return WC_NEXT;
  };
  RT len = calcVectLen(tstat.StDev);
  walk(signal, scale, &len);
}

static void adjustSignal(MatrixDyn &signal, SignalStat &tstat) {
  shift(signal, tstat);
  scale(signal, tstat);
}

static bool checkThreshold(const MatrixDyn &val, const MatrixDyn &eta,
                           const MatrixDyn &stdv, float factor) {
  auto check = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    return mat(row, col) >= 0 ? WC_NEXT : WC_STOP;
  };

  MatrixDyn diff = val - (eta - stdv * factor / 2.f);
  if (!walk(diff, check, NULL))
    return false;

  diff = (eta + stdv * factor / 2.f) - val;
  if (!walk(diff, check, NULL))
    return false;

  return true;
}

int VibroGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  if (category <= -1 || category >= train_stats_.size()) {
    train_stats_.push_back({});
  }
  auto &tstat = train_stats_.back();
  tstat.calcSignal(input, hp.SeparateInaccuracies);
  adjustSignal(input, tstat);
  const auto vec = saveToVect(input);
  return grc_.train(vec.size(), vec.data(), category);
}

int VibroGrc::inference(const MatrixDyn &signal) {
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

unsigned VibroGrc::save(std::vector<RT> &data) {
  const auto qty = BaseGrc::save(data);

  for (const auto &tstat : train_stats_) {
    data.insert(data.end(), tstat.Mean.begin(),
                tstat.Mean.end());
    data.insert(data.end(), tstat.StDev.begin(),
                tstat.StDev.end());
  }

  return qty;
}

bool VibroGrc::load(unsigned qty, const std::vector<RT> &data) {
  const auto _data = loadSignalStat(qty, data);
  if (BaseGrc::load(qty, _data)) {
    return true;
  }
  return false;
}

std::vector<RT> VibroGrc::loadSignalStat(unsigned qty,
                                         const std::vector<RT> &data) {
  LOGD(name_, "load train stat");
  auto load = [](MatrixDyn &mat, unsigned rows, unsigned cols,
                 std::vector<RT>::const_iterator it) {
    const size_t size = rows * cols;
    const auto end = std::next(it, size);
    mat = MatrixDyn(it, end);
    mat.reshape(rows, cols);
    return end;
  };

  train_stats_.clear();
  const auto rows = hp.SeparateInaccuracies ? hp.InputComponents : 1;
  const auto cols = 1;
  // skip weights
  const auto wgt_end = data.end() - qty * rows * cols * 2;
  auto it = wgt_end;
  for (unsigned i = 0; i < qty; i++) {
    train_stats_.push_back({});
    auto &tstat = train_stats_.back();
    // construct matrix and move iterator to end
    it = load(tstat.Mean, rows, cols, it);
    it = load(tstat.StDev, rows, cols, it);
  }
  if (it != data.end()) {
    LOGE(name_, "Error loading train stats");
  }
  return std::vector<RT>(data.begin(), wgt_end);
}

int VibroGrc::clear() {
  train_stats_.clear();
  return BaseGrc::clear();
}

void VibroGrc::preprocess(MatrixDyn &signal, InferStat &istat,
                          SignalStat &tstat) const {
  LOGD(TAG, __FUNCTION__);

  istat.calcSignal(signal, hp.SeparateInaccuracies);

  rotate(signal, istat, tstat);

  istat.fail = 0;
  if (!checkThreshold(istat.Mean, tstat.Mean,
                      tstat.StDev, float(hp.ThresholdFactor))) {
    istat.mean_fail = true;
  }
  if (!checkThreshold(istat.StDev, tstat.StDev,
                      tstat.StDev, float(hp.ThresholdFactor))) {
    istat.stdev_fail = true;
  }

  adjustSignal(signal, tstat);
}

void VibroGrc::rotate(MatrixDyn &signal, InferStat &istat,
                      SignalStat &tstat) const {
  const RT coef = 1;
  MatrixDyn rot = coef * tstat.Mean - istat.Mean;
  rot = repeat(rot, 1, signal.numCols());
  signal += rot;

  istat.calcSignal(signal, hp.SeparateInaccuracies);
}
