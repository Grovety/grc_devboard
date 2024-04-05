#pragma once

#include <algorithm>

template<typename DT>
class MatrixSVD
{
private:
  const unsigned m_M, m_N;
  MatrixDT<DT> &m_U, &m_V, &m_S;
  const DT m_Eps;
  DT m_Tsh;

public:
  explicit MatrixSVD(MatrixDT<DT> &U, MatrixDT<DT> &V, MatrixDT<DT> &S) :
    m_M(U.getRows()),
    m_N(U.getCols()),
    m_U(U),
    m_V(V),
    m_S(S),
    m_Eps(DT::getEps())  // * 3 !!!
  {
    m_V.resize(m_N, m_N, false);
    m_S.resize(1, m_N, false);

    decompose();
    reorder();
    m_Tsh = (DT((int)(m_M + m_N + 1)).calcSqrt() * m_S(0, 0) * m_Eps) / 2;
  }

private:
  void reorder() {
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;
    uint32_t ss = 0;
    uint32_t inc = 1;

    DT sw = 0;
    MatrixDT<DT> su(m_M, 1);
    MatrixDT<DT> sv(m_N, 1);

    do
    {
      inc *= 3;
      ++inc;
    } while (inc <= m_N);

    do
    {
      inc /= 3;
      for (i = inc; i < m_N; ++i)
      {
        sw = m_S[i];
        for (k = 0; k < m_M; ++k)
        {
          su[k] = m_U(k, i);
        }

        for (k = 0; k < m_N; ++k)
        {
          sv[k] = m_V(k, i);
        }

        j = i;
        while (m_S[j - inc] < sw)
        {
          m_S[j] = m_S[j - inc];

          for (k = 0; k < m_M; ++k)
          {
            m_U(k, j) = m_U(k, j - inc);
          }

          for (k = 0; k < m_N; ++k)
          {
            m_V(k, j) = m_V(k, j - inc);
          }

          j -= inc;

          if (j < inc)
          {
            break;
          }
        }

        m_S[j] = sw;

        for (k = 0; k < m_M; ++k)
        {
          m_U(k, j) = su[k];
        }

        for (k = 0; k < m_N; ++k)
        {
          m_V(k, j) = sv[k];
        }
      }
    } while (inc > 1);

    for (k = 0; k < m_N; ++k)
    {
      ss = 0;

      for (i = 0; i < m_M; i++)
      {
        if (m_U(i, k).isNeg())
        {
          ss++;
        }
      }

      for (j = 0; j < m_N; ++j)
      {
        if (m_V(j, k).isNeg())
        {
          ss++;
        }
      }

      if (ss > (m_M + m_N) / 2)
      {
        for (i = 0; i < m_M; ++i)
        {
          m_U(i, k) = -m_U(i, k);
        }

        for (j = 0; j < m_N; ++j)
        {
          m_V(j, k) = -m_V(j, k);
        }
      }
    }
  }

  static DT SIGN(DT inA, DT inB) noexcept {
    return !inB.isNeg() ? (!inA.isNeg() ? inA : DT(-inA)) : (!inA.isNeg() ? DT(-inA) : inA);
  }

  void decompose() {
    bool flag = true;
    uint32_t i = 0;
    uint32_t its = 0;
    uint32_t j = 0;
    uint32_t jj = 0;
    uint32_t k = 0;
    uint32_t l = 0;
    uint32_t nm = 0;

    DT anorm = 0;
    DT c = 0;
    DT f = 0;
    DT g = 0;
    DT h = 0;
    DT ss = 0;
    DT scale = 0;
    DT x = 0;
    DT y = 0;
    DT z = 0;

    MatrixDT<DT> rv1(m_N, 1);

    for (i = 0; i < m_N; ++i)
    {
      l = i + 2;
      rv1[i] = scale * g;
      g = ss = scale = 0;

      if (i < m_M)
      {
        for (k = i; k < m_M; ++k)
        {
          scale += m_U(k, i).getAbs();
        }

        if (scale != 0)
        {
          for (k = i; k < m_M; ++k)
          {
            m_U(k, i) /= scale;
            ss += m_U(k, i) * m_U(k, i);
          }

          f = m_U(i, i);
          g = -SIGN(ss.calcSqrt(), f);
          h = f * g - ss;
          m_U(i, i) = f - g;

          for (j = l - 1; j < m_N; ++j)
          {
            for (ss = 0, k = i; k < m_M; ++k)
            {
              ss += m_U(k, i) * m_U(k, j);
            }

            f = ss / h;

            for (k = i; k < m_M; ++k)
            {
              m_U(k, j) += f * m_U(k, i);
            }
          }

          for (k = i; k < m_M; ++k)
          {
            m_U(k, i) *= scale;
          }
        }
      }

      m_S[i] = scale * g;
      g = ss = scale = 0;

      if (i + 1 <= m_M && i + 1 != m_N)
      {
        for (k = l - 1; k < m_N; ++k)
        {
          scale += m_U(i, k).getAbs();
        }

        if (scale != 0)
        {
          for (k = l - 1; k < m_N; ++k)
          {
            m_U(i, k) /= scale;
            ss += m_U(i, k) * m_U(i, k);
          }

          f = m_U(i, l - 1);
          g = -SIGN(ss.calcSqrt(), f);
          h = f * g - ss;
          m_U(i, l - 1) = f - g;

          for (k = l - 1; k < m_N; ++k)
          {
            rv1[k] = m_U(i, k) / h;
          }

          for (j = l - 1; j < m_M; ++j)
          {
            for (ss = 0, k = l - 1; k < m_N; ++k)
            {
              ss += m_U(j, k) * m_U(i, k);
            }

            for (k = l - 1; k < m_N; ++k)
            {
              m_U(j, k) += ss * rv1[k];
            }
          }

          for (k = l - 1; k < m_N; ++k)
          {
            m_U(i, k) *= scale;
          }
        }
      }

      anorm = std::max(anorm, DT(m_S[i].getAbs() + rv1[i].getAbs()));
    }

    for (i = m_N - 1; i != static_cast<uint32_t>(-1); --i)
    {
      if (i < m_N - 1)
      {
        if (g != 0)
        {
          for (j = l; j < m_N; ++j)
          {
            m_V(j, i) = (m_U(i, j) / m_U(i, l)) / g;
          }

          for (j = l; j < m_N; ++j)
          {
            for (ss = 0, k = l; k < m_N; ++k)
            {
              ss += m_U(i, k) * m_V(k, j);
            }

            for (k = l; k < m_N; ++k)
            {
              m_V(k, j) += ss * m_V(k, i);
            }
          }
        }

        for (j = l; j < m_N; ++j)
        {
          m_V(i, j) = m_V(j, i) = 0;
        }
      }

      m_V(i, i) = 1;
      g = rv1[i];
      l = i;
    }

    for (i = std::min(m_M, m_N) - 1; i != static_cast<uint32_t>(-1); --i)
    {
      l = i + 1;
      g = m_S[i];

      for (j = l; j < m_N; ++j)
      {
        m_U(i, j) = 0;
      }

      if (g != 0)
      {
        g = g.getInv();

        for (j = l; j < m_N; ++j)
        {
          for (ss = 0, k = l; k < m_M; ++k)
          {
            ss += m_U(k, i) * m_U(k, j);
          }

          f = (ss / m_U(i, i)) * g;

          for (k = i; k < m_M; ++k)
          {
            m_U(k, j) += f * m_U(k, i);
          }
        }

        for (j = i; j < m_M; ++j)
        {
          m_U(j, i) *= g;
        }
      }
      else
      {
        for (j = i; j < m_M; ++j)
        {
          m_U(j, i) = 0;
        }
      }

      m_U(i, i) = m_U(i, i) + 1;
    }

    for (k = m_N - 1; k != static_cast<uint32_t>(-1); --k)
    {
      for (its = 0; its < 30; ++its)
      {
        flag = true;
        for (l = k; l != static_cast<uint32_t>(-1); --l)
        {
          nm = l - 1;
          if (l == 0 || rv1[l].getAbs() <= m_Eps * anorm)
          {
            flag = false;
            break;
          }

          if (m_S[nm].getAbs() <= m_Eps * anorm)
          {
            break;
          }
        }

        if (flag)
        {
          c = 0;
          ss = 1;
          for (i = l; i < k + 1; ++i)
          {
            f = ss * rv1[i];
            rv1[i] = c * rv1[i];

            if (f.getAbs() <= m_Eps * anorm)
            {
              break;
            }

            g = m_S[i];
            h = pythag(f, g);
            m_S[i] = h;
            h = h.getInv();
            c = g * h;
            ss = -f * h;

            for (j = 0; j < m_M; ++j)
            {
              y = m_U(j, nm);
              z = m_U(j, i);
              m_U(j, nm) = y * c + z * ss;
              m_U(j, i) = z * c - y * ss;
            }
          }
        }

        z = m_S[k];
        if (l == k)
        {
          if (z < 0)
          {
            m_S[k] = -z;
            for (j = 0; j < m_N; ++j)
            {
              m_V(j, k) = -m_V(j, k);
            }
          }
          break;
        }

        if (its == 29)
        {
          //!!!THROW_INVALID_ARGUMENT_ERROR("no convergence in 30 svdcmp iterations");
          break;
        }

        x = m_S[l];
        nm = k - 1;
        y = m_S[nm];
        g = rv1[nm];
        h = rv1[k];
        f = ((y - z) * (y + z) + (g - h) * (g + h)) / (h * y * 2);
        g = pythag(f, 1);
        f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
        c = ss = 1;

        for (j = l; j <= nm; j++)
        {
          i = j + 1;
          g = rv1[i];
          y = m_S[i];
          h = ss * g;
          g = c * g;
          z = pythag(f, h);
          rv1[j] = z;
          c = f / z;
          ss = h / z;
          f = x * c + g * ss;
          g = g * c - x * ss;
          h = y * ss;
          y *= c;

          for (jj = 0; jj < m_N; ++jj)
          {
            x = m_V(jj, j);
            z = m_V(jj, i);
            m_V(jj, j) = x * c + z * ss;
            m_V(jj, i) = z * c - x * ss;
          }

          z = pythag(f, h);
          m_S[j] = z;

          if (z != 0)
          {
            z = z.getInv();
            c = f * z;
            ss = h * z;
          }

          f = c * g + ss * y;
          x = c * y - ss * g;

          for (jj = 0; jj < m_M; ++jj)
          {
            y = m_U(jj, j);
            z = m_U(jj, i);
            m_U(jj, j) = y * c + z * ss;
            m_U(jj, i) = z * c - y * ss;
          }
        }
        rv1[l] = 0;
        rv1[k] = f;
        m_S[k] = x;
      }
    }
  }

  static DT pythag(DT inA, DT inB) noexcept {
    const DT absa = inA.getAbs();
    const DT absb = inB.getAbs();
    return (absa > absb ? absa * DT((absb / absa)*(absb / absa) + 1).calcSqrt()
                        : (absb == 0 ? 0 : absb * DT((absa / absb)*(absa / absb) + 1).calcSqrt()));
  }
};
