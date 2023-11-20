#include "assert.h"

#include "common.hpp"
#include "numbers.hpp"
#include "preprocessing.hpp"
#include "statistics.hpp"

#define _ASSERT(e)

constexpr char TAG[] = "preprocessing";

enum WALK_CMD { WC_NEXT, WC_SKIP, WC_STOP };

typedef WALK_CMD (*WalkFuncDyn)(MatrixDyn &mat, unsigned row, unsigned col,
                                void *parm);

static bool walk(MatrixDyn &mat, WalkFuncDyn func, void *parm = nullptr) {
  for (unsigned row = 0; row < mat.numRows(); ++row) {
    for (unsigned col = 0; col < mat.numCols(); ++col) {
      WALK_CMD cmd = (*func)(mat, row, col, parm);
      if (cmd == WC_STOP)
        return false;
      if (cmd == WC_SKIP)
        break;
      _ASSERT(cmd == WC_NEXT && "Bad WALK_CMD!");
    }
  }
  return true;
}

void concat(MatrixDyn &dst, const MatrixDyn &src) {
  _ASSERT(src.numRows() == dst.numRows());

  unsigned keep = dst.numCols();
  if (keep)
    dst.resizeSlow(dst.numRows(), keep + src.numCols());
  else
    dst.resizeFast(dst.numRows(), src.numCols());

  dst.put(dst.rSlice(), nc::Slice(keep, keep + src.numCols()), src);
}

void smooth(MatrixDyn &dst, const MatrixDyn &src, unsigned avg_n,
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

static void resize(MatrixDyn &mat, unsigned rows, unsigned cols) {
  if (mat.numRows() && mat.numCols())
    mat.resizeSlow(rows, cols);
  else
    mat.resizeFast(rows, cols);
}
static void resizeAdd(MatrixDyn &mat, unsigned rows, unsigned cols) {
  resize(mat, mat.numRows() + rows, mat.numCols() + cols);
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

static RT calcVectLen(const MatrixDyn &vec) {
  _ASSERT(vec.getCols() == 1);
  auto sumSq = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    const RT val = mat(row, col);
    *(RT *)parm += val * val;
    return WC_NEXT;
  };
  RT sum = 0;
  walk(const_cast<MatrixDyn &>(vec), sumSq, &sum);
  return sqrt(sum);
}

static void shift(MatrixDyn &signal, TrainStat &tstat) {
  auto shift = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) -= (*(MatrixDyn *)parm)(row, 0);
    return WC_NEXT;
  };
  walk(signal, shift, &tstat.m_SignalMean);
}

static void scale(MatrixDyn &signal, TrainStat &tstat) {
  auto scale = [](MatrixDyn &mat, unsigned row, unsigned col, void *parm) {
    mat(row, col) /= 2 * *(RT *)parm;
    return WC_NEXT;
  };
  RT len = calcVectLen(tstat.m_SignalStDev);
  walk(signal, scale, &len);
}

void adjustSignal(MatrixDyn &signal, TrainStat &tstat) {
  shift(signal, tstat);
  scale(signal, tstat);
}

static void rotate(MatrixDyn &signal, TrainStat &tstat, InferStat &istat,
                   const HP &hp) {
  const RT coef = 1;
  MatrixDyn rot = coef * tstat.m_SignalMean - istat.m_SignalMean;
  rot = repeat(rot, 1, signal.numCols());
  signal += rot;

  istat.calcSignal(signal, hp.m_SeparateInaccuracies);
}

void preprocessVibro(MatrixDyn &signal, InferStat& istat, TrainStat &tstat, const HP &hp) {
  LOGD(TAG, __FUNCTION__);

  istat.calcSignal(signal, hp.m_SeparateInaccuracies);

  rotate(signal, tstat, istat, hp);

  istat.m_Fail = 0;
  if (!checkThreshold(istat.m_SignalMean, tstat.m_SignalMean,
                      tstat.m_SignalStDev, float(hp.m_ThresholdFactor))) {
    istat.m_MeanFail = true;
  }
  if (!checkThreshold(istat.m_SignalStDev, tstat.m_SignalStDev,
                      tstat.m_SignalStDev, float(hp.m_ThresholdFactor))) {
    istat.m_AmplFail = true;
  }

  adjustSignal(signal, tstat);
}
