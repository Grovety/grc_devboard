#pragma once
#include "assert.h"

#define _ASSERT(expr)  assert(expr)
#include "Numbers.hpp"
#include "Matrix.hpp"

typedef MatrixDT<FloatNum<float>> Matrix;

struct SignalData_t {
  float *buffer = nullptr;
  unsigned num_rows = 0;
  unsigned num_cols = 0;
  unsigned sync_time_ms = 0;
};

/*!
 * \brief Signal statistics.
 */
struct SignalStat {
  Matrix Mean;
  Matrix StDev;
  /*!
   * \brief Calculate statistics.
   * \param signal Input signal.
   * \param sep_inacc Separate inaccuracies.
   */
  void calcSignal(const Matrix &signal, bool sep_inacc) {
    signal.calcStat(Mean, StatMMAS<Matrix::ElmType>::Avg, false, sep_inacc);
    signal.calcStat(StDev, StatMMAS<Matrix::ElmType>::StDev, false, sep_inacc);
  }
};
/*!
 * \brief Infer signal statistics.
 */
struct InferStat : SignalStat {
  union {
    unsigned fail;
    struct {
      unsigned mean_fail : 1;
      unsigned stdev_fail : 1;
    };
  };

  InferStat() { fail = 0; }
};