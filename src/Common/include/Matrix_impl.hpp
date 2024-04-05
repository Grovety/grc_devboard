#pragma once

template<typename DT>
void MatrixDT<DT>::setVal(DT val) {
  for (DT* pdst = m_Ptr, *pend = m_Ptr + getElms(); pdst < pend; ++pdst)
    *pdst = val;
}

inline double rnd() { return rand() / (RAND_MAX + 1.0); }

template<typename DT>
void MatrixDT<DT>::addRand(DT ampl) {
  for (DT* pdst = m_Ptr, *pend = m_Ptr + getElms(); pdst < pend; ++pdst)
    *pdst += ampl * (2 * DT(::rnd()) - 1);
}

template<typename DT>
MatrixDT<DT> &MatrixDT<DT>::add(const MatrixDT &mat1, const MatrixDT &mat2) {
  unsigned rows = mat1.getRows(), cols = mat1.getCols();
  _ASSERT(mat2.getRows() == rows && mat2.getCols() == cols);
  resize(rows, cols, false);
  if (m_Trans == mat1.m_Trans && m_Trans == mat2.m_Trans) {
    const DT *psrc1 = mat1.m_Ptr, *psrc2 = mat2.m_Ptr;
    for (DT* pdst = m_Ptr, *pend = m_Ptr + getElms(); pdst < pend;)
      *pdst++ = *psrc1++ + *psrc2++;
  } else {
    for (unsigned row = 0; row < rows; ++row)
      for (unsigned col = 0; col < cols; ++col)
        get(row, col) = mat1(row, col) + mat2(row, col);
  }
  return *this;
}

template<typename DT>
MatrixDT<DT> &MatrixDT<DT>::sub(const MatrixDT &mat1, const MatrixDT &mat2) {
  unsigned rows = mat1.getRows(), cols = mat1.getCols();
  _ASSERT(mat2.getRows() == rows && mat2.getCols() == cols);
  resize(rows, cols, false);
  if (m_Trans == mat1.m_Trans && m_Trans == mat2.m_Trans) {
    const DT *psrc1 = mat1.m_Ptr, *psrc2 = mat2.m_Ptr;
    for (DT* pdst = m_Ptr, *pend = m_Ptr + getElms(); pdst < pend;)
      *pdst++ = *psrc1++ - *psrc2++;
  }
  else {
    for (unsigned row = 0; row < rows; ++row)
      for (unsigned col = 0; col < cols; ++col)
        get(row, col) = mat1(row, col) - mat2(row, col);
  }
  return *this;
}

template<typename DT>
MatrixDT<DT> &MatrixDT<DT>::mul(const MatrixDT &mat, DT val) {
  unsigned rows = mat.getRows(), cols = mat.getCols();
  resize(rows, cols, false);
  if (m_Trans == mat.m_Trans) {
    const DT *psrc = mat.m_Ptr;
    for (DT* pdst = m_Ptr, *pend = m_Ptr + getElms(); pdst < pend;)
      *pdst++ = *psrc++ * val;
  }
  else {
    for (unsigned row = 0; row < rows; ++row)
      for (unsigned col = 0; col < cols; ++col)
        get(row, col) = val * mat(row, col);
  }
  return *this;
}

template<typename DT>
MatrixDT<DT> MatrixDT<DT>::calcFunc(DT(*pfunc)(DT)) const {
  MatrixDT res(getRows(), getCols());
  const DT *psrc = m_Ptr;
  for (DT *pdst = res.m_Ptr, *pend = res.m_Ptr + getElms(); pdst < pend; )
    *pdst++ = (*pfunc)(*psrc++);
  return res;
}

template<typename DT>
void MatrixDT<DT>::calcStat(MatrixDT &res, enum StatMMAS<DT>::Kind kind, bool ByAbs, bool ByRows) const {
  res.resize(ByRows ? getRows() : 1, 1, false);

  StatMMAS<DT> stat;
  for (unsigned row = 0; row < getRows(); ++row) {
    if (ByRows)
      stat.clear();
    for (unsigned col = 0; col < getCols(); ++col) {
      DT val = ByAbs ? get(row, col).getAbs() : get(row, col);
      stat.put(val);
    }
    if (ByRows)
      res(row, 0) = stat.getStat(kind);
  }
  if (!ByRows)
    res(0, 0) = stat.getStat(kind);
}

template<typename DT>
void MatrixDT<DT>::resize(unsigned rows, unsigned cols, bool keep, bool zero, bool front) {
  unsigned elms = rows * cols;
  if (!elms) {
    free();
    init(rows, cols);
    return;
  }

  if (!getElms()) {
    *this = MatrixDT(rows, cols, zero);
    return;
  }

  if (isSameShape(rows, cols))
    return;

  _ASSERT(!m_Const && !m_Alien);

  _ASSERT(!zero);  //!!!

  if (m_Trans)
    std::swap(rows, cols);

  if (elms > getElms())
    reallocElms(getElms(), elms, cols == m_BlkLen);

  if (keep) {
    unsigned rRem = std::min(m_BlkQty, rows);

    if (cols < m_BlkLen) {
      DT *pDst = m_Ptr, *pSrc = m_Ptr + (!front ? 0 : m_BlkLen - cols);
      for (unsigned i = 0; i < rRem; ++i) {
        memmove((void *)pDst, pSrc, cols * sizeof(DT));
        pDst += cols;
        pSrc += m_BlkLen;
      }
    } else if (cols > m_BlkLen) {
      DT *pDst = m_Ptr + (!front ? 0 : cols - m_BlkLen) + elms, *pSrc = m_Ptr + getElms();
      for (unsigned i = 0; i < rRem; ++i) {
        pDst -= cols;
        pSrc -= m_BlkLen;
        memmove((void *)pDst, pSrc, m_BlkLen * sizeof(DT));
      }
    } else if (front && rows != m_BlkQty) {
      DT *pDst = m_Ptr + m_BlkLen * (rows < m_BlkQty ? 0 : rows - m_BlkQty);
      DT *pSrc = m_Ptr + m_BlkLen * (rows < m_BlkQty ? m_BlkQty - rows : 0);
      memmove((void *)pDst, pSrc, m_BlkLen * rRem * sizeof(DT));
    }
  }

  if (elms < getElms())
    reallocElms(getElms(), elms);

  m_BlkLen = cols;
  m_BlkQty = rows;
}

template<typename DT>
void MatrixDT<DT>::concatCols(const MatrixDT &mat) {
  _ASSERT(&mat != this && getRows() == mat.getRows());

  unsigned cols = getCols();
  resize(getRows(), cols + mat.getCols(), true);

  set(0, cols, mat);
}
template<typename DT>
void MatrixDT<DT>::concatRows(const MatrixDT &mat) {
  _ASSERT(&mat != this && getCols() == mat.getCols());

  unsigned rows = getRows();
  resize(rows + mat.getRows(), getCols(), true);

  set(rows, 0, mat);
}

template<typename DT>
MatrixDT<DT> MatrixDT<DT>::repeat(unsigned nrows, unsigned ncols) const {
  MatrixDT res(nrows * getRows(), ncols * getCols());

  for (unsigned ccnt = 0; ccnt < ncols; ++ccnt)
    res.set(0, ccnt * getCols(), *this);

  for (unsigned rcnt = 1; rcnt < nrows; ++rcnt)
    res.set(rcnt * res.getCols(), 0, res);

  return res;
}

template<typename DT>
std::vector<DT> MatrixDT<DT>::getAsVector() const {
  std::vector<DT> res;

  for (unsigned row = 0; row < getRows(); ++row)
    for (unsigned col = 0; col < getCols(); ++col)
      res.push_back(get(row, col));

  return res;
}

template<typename DT>
MatrixDT<DT> MatrixDT<DT>::buildDiagMat(int k) const {
  const auto absK = static_cast<uint32_t>(std::abs(k));
  MatrixDT res(getElms() + absK, getElms() + absK, true);

  const uint32_t rowOffset = k < 0 ? absK : 0;
  const uint32_t colOffset = k > 0 ? absK : 0;

  const DT *rptr = getRowPtr(0);
  _ASSERT(rptr); //!!!
  for (unsigned d = 0; d < getElms(); ++d)
    res(d + rowOffset, d + colOffset) = *rptr++;

  return res;
}

template<typename DT>
void MatrixDT<DT>::print(const char *name, char space) const {
  if (name)
    printf("%s:\n", name);

  for (unsigned row = 0; row < getRows(); ++row) {
    for (unsigned col = 0; col < getCols(); ++col) {
      printf("% 4.4f", (float)get(row, col));
      if (col + 1 < getCols())
        printf("%c", space);
    }
    printf("\n");
  }
}

template<typename DT>
bool MatrixDT<DT>::Rect::recalc(unsigned rows, unsigned cols) {
  if (m_RBeg < 0)
    m_RBeg = rows - -m_RBeg;
  if (m_CBeg < 0)
    m_CBeg = cols - -m_CBeg;

  bool is_ok = (m_RBeg >= 0 && (unsigned)m_RBeg <= rows && m_CBeg >= 0 && (unsigned)m_CBeg <= cols);
  _ASSERT(is_ok && "Rect Beg");
  if (!is_ok)
    return false;

  if (m_RQty == EOM)
    m_RQty = rows - m_RBeg;
  else if (m_RQty < 0)
    m_RQty = (rows - m_RBeg) - -m_RQty;

  if (m_CQty == EOM)
    m_CQty = cols - m_CBeg;
  else if (m_CQty < 0)
    m_CQty = (cols - m_CBeg) - -m_CQty;

  is_ok = (m_RQty >= 0 && (unsigned)(m_RBeg + m_RQty) <= rows && m_CQty >= 0 && (unsigned)(m_CBeg + m_CQty) <= cols);
  _ASSERT(is_ok && "Rect Qty");

  return is_ok;
}
