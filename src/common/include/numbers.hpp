#pragma once

#include <float.h>
#include <math.h>
#include <stdio.h>

template <typename DT> class FloatNum {
private:
  DT m_val;

public:
  FloatNum() {}
  FloatNum(const FloatNum &v) { m_val = v.m_val; }
  FloatNum(double val) { m_val = (DT)val; }

  operator const DT &() const { return m_val; }
  operator DT &() { return m_val; }

  FloatNum &operator=(const FloatNum &v) {
    m_val = v.m_val;
    return *this;
  }

  DT get() const { return m_val; }
  FloatNum set(DT val) {
    m_val = val;
    return *this;
  }

  static FloatNum getMin() { return -FLT_MAX; }    //!!!
  static FloatNum getMax() { return +FLT_MAX; }    //!!!
  static FloatNum getEps() { return FLT_EPSILON; } //!!!

  bool isNeg() const { return m_val < 0; }

  FloatNum getAbs() const { return m_val >= 0 ? m_val : -m_val; }
  FloatNum getInv() const { return 1. / m_val; }
  FloatNum calcSqrt() const { return sqrt(m_val); }

  void print(bool newline = false, bool brief = false) const {
    printf("% 4.4f", get());
    if (newline)
      printf("\n");
  }
};

template <typename DT> inline FloatNum<DT> calcTanH(FloatNum<DT> n) {
  return ::tanh(n.get());
}
