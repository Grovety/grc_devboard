#include "Statistics.hpp"

using namespace nc::linalg;

void SignalStat::calcSignal(const MatrixDyn &signal, bool sep_inacc) {
  Mean =
      (sep_inacc ? mean(signal, nc::Axis::COL).transpose() : mean(signal))
          .astype<float>();
  StDev =
      (sep_inacc ? stdev(signal, nc::Axis::COL).transpose() : stdev(signal))
          .astype<float>();
}
