#include "custom_types.hpp"

using namespace nc::linalg;

void SignalStat::calcSignal(const MatrixDyn &signal, bool sep_inacc) {
  m_SignalMean =
      (sep_inacc ? mean(signal, nc::Axis::COL).transpose() : mean(signal))
          .astype<float>();
  m_SignalStDev =
      (sep_inacc ? stdev(signal, nc::Axis::COL).transpose() : stdev(signal))
          .astype<float>();
}

void SignalStat::calcInacc(const MatrixDyn &inacc, bool sep_inacc) {
  m_InaccAbsMean = (sep_inacc ? mean(abs(inacc), nc::Axis::COL).transpose()
                              : mean(abs(inacc)))
                       .astype<float>();
  m_InaccStDev =
      (sep_inacc ? stdev(inacc, nc::Axis::COL).transpose() : stdev(inacc))
          .astype<float>();
}
