#include "VibroGrc.hpp"
#include "preprocessing.hpp"

int VibroGrc::train(const MatrixDyn &signal, int category) {
  auto input = signal;
  if (category <= -1 || category >= train_stats_.size()) {
    train_stats_.push_back({});
  }
  auto& tstat = train_stats_.back();
  tstat.calcSignal(input, hyper_params_.m_SeparateInaccuracies);
  adjustSignal(input, tstat);
  const auto vec = saveToVect(input);
  return trainReq(vec.size(), vec.data(), category);
}

int VibroGrc::inference(const MatrixDyn &signal) {
  // use train stat info for each category to preprocess the signal
  for (int i = 0; i < train_stats_.size(); ++i) {
    auto input = signal;

    InferStat istat;
    preprocessVibro(input, istat, train_stats_[i], hyper_params_);
    if (istat.m_Fail) {
      continue;
    }
    const auto vec = saveToVect(input);

    const auto category = inferenceReq(vec.size(), vec.data(), i);
    if (category > -1) {
      return category;
    }
  }
  return -1;
}

unsigned VibroGrc::save(std::vector<RT> &data) {
  const auto qty = BaseGrc::save(data);

  for (const auto& tstat : train_stats_) {
    data.insert(data.end(), tstat.m_SignalMean.begin(), tstat.m_SignalMean.end());
    data.insert(data.end(), tstat.m_SignalStDev.begin(), tstat.m_SignalStDev.end());
  }

  return qty;
}

bool VibroGrc::load(unsigned qty, const std::vector<RT> &data) {
  const auto _data = loadTrainStat(qty, data);
  if (BaseGrc::load(qty, _data)) {
    return true;
  }
  return false;
}

std::vector<RT> VibroGrc::loadTrainStat(unsigned qty, const std::vector<RT> &data) {
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
  const auto rows = hyper_params_.m_SeparateInaccuracies
                        ? hyper_params_.m_InputComponents
                        : 1;
  const auto cols = 1;
  // skip weights
  const auto wgt_end = data.end() - qty * rows * cols * 2;
  auto it = wgt_end;
  for (unsigned i = 0; i < qty; i++) {
    train_stats_.push_back({});
    auto &tstat = train_stats_.back();
    // construct matrix and move iterator to end
    it = load(tstat.m_SignalMean, rows, cols, it);
    it = load(tstat.m_SignalStDev, rows, cols, it);
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