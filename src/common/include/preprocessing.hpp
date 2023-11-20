#pragma once
#include "custom_types.hpp"

void smooth(MatrixDyn &dst, const MatrixDyn &src, unsigned avg_n,
            unsigned comps, unsigned beg);
void concat(MatrixDyn &dst, const MatrixDyn &src);
bool preprocessRhythm(MatrixDyn &signal, unsigned signal_period_ms);
void preprocessVibro(MatrixDyn &signal, InferStat& istat, TrainStat &tstat, const HP &hp);
void adjustSignal(MatrixDyn &signal, TrainStat &tstat);
