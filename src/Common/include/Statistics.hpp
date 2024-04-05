#pragma once

#include <stdio.h>
#include "Common.hpp"

/*!
 * \brief Helper class to compute statistics.
 */
template <typename DT> class StatMMAS {
public:
  enum Kind { Sum, Min, Max, Avg, StDev };

private:
  unsigned cnt_;
  DT sum_;
  DT sqrs_;
  DT min_, max_;

public:
  StatMMAS() { clear(); }

  void clear() {
    cnt_ = 0;
    sum_ = 0;
    sqrs_ = 0;
    min_ = DT::getMax();
    max_ = DT::getMin();
  }

  void put(DT val) {
    sum_ += val;
    sqrs_ += val * val;
    if (val < min_)
      min_ = val;
    if (val > max_)
      max_ = val;
    ++cnt_;
  }

  void put(const StatMMAS st) {
    sum_ += st.sum_;
    sqrs_ += st.sqrs_;
    if (st.min_ < min_)
      min_ = st.min_;
    if (st.max_ > st.max_)
      max_ = st.max_;
    cnt_ += st.cnt_;
  }

  unsigned getCount() const { return cnt_; }
  DT getSum() const { return sum_; }
  DT getMin() const { return min_; }
  DT getMax() const { return max_; }
  DT getAvg() const { return cnt_ ? DT(getSum() / cnt_) : DT(0); }
  DT getStDev() const {
    return cnt_ ? DT(sqrs_ / cnt_ - getAvg() * getAvg()).calcSqrt() : DT(0);
  }

  DT getStat(Kind kind) const {
    switch (kind) {
    case Sum:
      return getSum();
    case Min:
      return getMin();
    case Max:
      return getMax();
    case Avg:
      return getAvg();
    case StDev:
      return getStDev();
    }
    return 0;
  }
};
