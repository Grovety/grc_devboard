#pragma once
#include "NumCpp.hpp"

typedef float RT;
typedef nc::NdArray<RT> MatrixDyn;

struct SignalData_t {
  float *buffer = nullptr;
  unsigned num_rows = 0;
  unsigned num_cols = 0;
  unsigned sync_time_ms = 0;
};
