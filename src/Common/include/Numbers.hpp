#pragma once

#include <float.h>
#include <math.h>
#include <stdio.h>

/*!
 * \brief Floating point number wrapper.
 */
template <typename DT> class FloatNum {
private:
  DT val_;

public:
  FloatNum() {}
  FloatNum(const FloatNum &v) { val_ = v.val_; }
  FloatNum(double val) { val_ = (DT)val; }

  operator const DT &() const { return val_; }
  operator DT &() { return val_; }

  FloatNum &operator=(const FloatNum &v) {
    val_ = v.val_;
    return *this;
  }

  DT get() const { return val_; }
  FloatNum set(DT val) {
    val_ = val;
    return *this;
  }

  static FloatNum getMin() { return -FLT_MAX; }    //!!!
  static FloatNum getMax() { return +FLT_MAX; }    //!!!
  static FloatNum getEps() { return FLT_EPSILON; } //!!!

  bool isNeg() const { return val_ < 0; }

  FloatNum getAbs() const { return val_ >= 0 ? val_ : -val_; }
  FloatNum getInv() const { return 1. / val_; }
  FloatNum calcSqrt() const { return sqrt(val_); }
};
