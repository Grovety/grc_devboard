#pragma once
#include "NumCpp.hpp"

typedef float /*float*/ RT;
typedef nc::NdArray<RT> MatrixDyn;

struct HP {
  bool m_PredictSignal;
  bool m_SeparateInaccuracies;
  unsigned m_InputComponents;
  unsigned m_OutputComponents;
  unsigned m_ReservoirNeurons;
  double m_SpectralRadius;
  double m_Sparsity;
  double m_Noise;
  double m_InputScaling;
  double m_InputSparsity;
  double m_FeedbackScaling;
  double m_FeedbackSparsity;
  double m_ThresholdFactor;
};

struct SignalStat {
  MatrixDyn m_SignalMean;
  MatrixDyn m_SignalStDev;

  MatrixDyn m_InaccAbsMean;
  MatrixDyn m_InaccStDev;

  void calcSignal(const MatrixDyn &signal, bool sep_inacc);
  void calcInacc(const MatrixDyn &inacc, bool sep_inacc);
};
struct TrainStat : SignalStat {
  unsigned m_InputQty, m_UsedQty;
};
struct InferStat : SignalStat {
  union {
    unsigned m_Fail;
    struct {
      unsigned m_MeanFail : 1;
      unsigned m_AmplFail : 1;
    };
  };

  InferStat() { m_Fail = 0; }
};

struct SignalData_t {
  float *buffer = nullptr;
  unsigned num_rows = 0;
  unsigned num_cols = 0;
  unsigned sync_time_ms = 0;
};
