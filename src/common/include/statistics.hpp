#pragma once

#include <stdio.h>

template <typename DT> class StatMMAS {
public:
  enum Kind { Sum, Min, Max, Avg, StDev };

private:
  unsigned m_cnt;
  DT m_sum;
  DT m_sqrs;
  DT m_min, m_max;

public:
  StatMMAS() { clear(); }

  void clear() {
    m_cnt = 0;
    m_sum = 0;
    m_sqrs = 0;
    m_min = DT::getMax();
    m_max = DT::getMin();
  }

  void put(DT val) {
    m_sum += val;
    m_sqrs += val * val;
    if (val < m_min)
      m_min = val;
    if (val > m_max)
      m_max = val;
    ++m_cnt;
  }

  void put(const StatMMAS st) {
    m_sum += st.m_sum;
    m_sqrs += st.m_sqrs;
    if (st.m_min < m_min)
      m_min = st.m_min;
    if (st.m_max > st.m_max)
      m_max = st.m_max;
    m_cnt += st.m_cnt;
  }

  unsigned getCount() const { return m_cnt; }
  DT getSum() const { return m_sum; }
  DT getMin() const { return m_min; }
  DT getMax() const { return m_max; }
  DT getAvg() const { return m_cnt ? DT(getSum() / m_cnt) : DT(0); }
  DT getStDev() const {
    return m_cnt ? DT(m_sqrs / m_cnt - getAvg() * getAvg()).calcSqrt() : DT(0);
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
