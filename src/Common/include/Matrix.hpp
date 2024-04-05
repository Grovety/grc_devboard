#pragma once

#include <limits.h>
#include <string.h>

#include <vector>

#include "Statistics.hpp"

template<typename DT>
class MatrixDT
{
public:
  typedef DT ElmType;
  static const int EOM = INT_MIN;
  struct Rect {
    int m_RBeg, m_CBeg, m_RQty, m_CQty;
    Rect(int rbeg = 0, int cbeg = 0, int rqty = EOM, int cqty = EOM) { m_RBeg = rbeg; m_CBeg = cbeg; m_RQty = rqty; m_CQty = cqty; }
    bool recalc(unsigned rows, unsigned cols);
    Rect swap() const { return Rect(m_CBeg, m_RBeg, m_CQty, m_RQty); }
  };
public:
  MatrixDT() { init(0, 0); }
  MatrixDT(unsigned rows, unsigned cols, bool zero = false) {
    init(rows, cols);
    allocElms(getElms(), zero);
  }
  MatrixDT(DT *ptr, unsigned rows, unsigned cols, bool copy = true, bool take_ownership = false, bool by_cols = false) {
    if (by_cols)
      std::swap(rows, cols);
    init(rows, cols);

    if (copy) {
      allocElms(getElms());
      memcpy((void *)m_Ptr, ptr, getElms() * sizeof(DT));
    } else {
      m_Ptr = ptr;
      m_Alien = !take_ownership;
    }
  }
  MatrixDT(const DT *ptr, unsigned rows, unsigned cols, bool copy = true) :
    MatrixDT(const_cast<DT *>(ptr), rows, cols, copy)
  {
    if (!copy)
      m_Const = true;
  }
  MatrixDT(const MatrixDT &mat, bool copy = true) :
    MatrixDT(mat.m_Ptr, mat.m_BlkQty, mat.m_BlkLen, copy)
  {
    m_Trans = mat.m_Trans;
    if (!copy)
      m_Const = mat.m_Const;
  }
  MatrixDT(MatrixDT &&mat) {
    init(0, 0);
    *this = std::move(mat);
  }

  MatrixDT(const MatrixDT &mat, Rect rect, bool copy = false) {  //!!! copy
    rect.recalc(mat.getRows(), mat.getCols());
    if (!copy) {
      Rect r = !mat.m_Trans ? rect : rect.swap();
      if (r.m_CBeg == 0 && r.m_CQty == mat.m_BlkLen) {
        m_Ptr = mat.m_Ptr + mat.m_BlkLen * r.m_RBeg;
        m_BlkLen = mat.m_BlkLen;
        m_BlkQty = r.m_RQty;
        m_Trans = mat.m_Trans;
        m_Alien = true;
        m_Const = mat.m_Const;
        return;
      }
    }

    init(0, 0);
    resize(rect.m_RQty, rect.m_CQty, false);
    for (unsigned row = 0; row < (unsigned)rect.m_RQty; ++row)
      for (unsigned col = 0; col < (unsigned)rect.m_CQty; ++col)
        get(row, col) = mat(rect.m_RBeg + row, rect.m_CBeg + col);
  }

  ~MatrixDT() { free(); }

  unsigned getRows() const { return !m_Trans ? m_BlkQty : m_BlkLen; }
  unsigned getCols() const { return !m_Trans ? m_BlkLen : m_BlkQty; }
  unsigned numRows() const { return getRows(); }
  unsigned numCols() const { return getCols(); }
  unsigned getElms() const { return m_BlkQty * m_BlkLen; }
  bool isEmpty() const { return getElms() == 0; }

  bool isSameShape(unsigned rows, unsigned cols) const { return getRows() == rows && getCols() == cols; }
  bool isSameShape(const MatrixDT &mat) const { return isSameShape(mat.getRows(), mat.getCols()); }

  const DT &get(unsigned row, unsigned col) const {
    _ASSERT(row < getRows() && col < getCols());
    return !m_Trans ? getRC(row, col) : getCR(row, col);
  }
  DT &get(unsigned row, unsigned col) {
    _ASSERT(!m_Const);
    return const_cast<DT &>(const_cast<const MatrixDT *>(this)->get(row, col));
  }

  DT &operator()(unsigned row, unsigned col) { return get(row, col); }
  const DT &operator()(unsigned row, unsigned col) const { return get(row, col); }

  DT operator[](int ind) const { return m_Ptr[ind >= 0 ? ind : ind + getElms()]; }  // 1-D access
  DT &operator[](int ind) { return m_Ptr[ind >= 0 ? ind : ind + getElms()]; }

  MatrixDT getRow(unsigned row) const {
    _ASSERT(row < getRows());
    MatrixDT res(1, getCols());
    for (unsigned col = 0; col < getCols(); ++col)
      res(0, col) = get(row, col);
    return res;
  }
  MatrixDT getCol(unsigned col) const {
    _ASSERT(col < getCols());
    MatrixDT res(getRows(), 1);
    for (unsigned row = 0; row < getRows(); ++row)
      res(row, 0) = get(row, col);
    return res;
  }

  MatrixDT &operator = (MatrixDT &&mat) {
    free();
    m_Ptr = mat.m_Ptr;
    m_BlkLen = mat.m_BlkLen;
    m_BlkQty = mat.m_BlkQty;
    m_Trans = mat.m_Trans;
    m_Alien = mat.m_Alien;
    m_Const = mat.m_Const;

    mat.init(0, 0);

    return *this;
  }
  MatrixDT &operator = (const MatrixDT &mat) {
    return *this = MatrixDT(mat);
  }

  void setZero() { memset((void *)m_Ptr, '\0', getElms() * sizeof(DT)); }
  void setVal(DT val);

  MatrixDT dot(const MatrixDT &mat) const {
    _ASSERT(getCols() == mat.getRows());

    MatrixDT res(getRows(), mat.getCols());
    for (unsigned row = 0; row < getRows(); ++row)
      for (unsigned col = 0; col < mat.getCols(); ++col)
        res(row, col) = prod(*this, mat, row, col, getCols());
    return res;
  }
  MatrixDT dotTT(const MatrixDT &mat) const {
    const_cast<MatrixDT &>(mat).trans();
    const_cast<MatrixDT *>(this)->trans();
    MatrixDT res = mat.dot(*this);
    const_cast<MatrixDT *>(this)->trans();
    const_cast<MatrixDT &>(mat).trans();
    res.trans();
    return res;
  }
  void dotLet(MatrixDT &mat) const {
    _ASSERT(getCols() == mat.getRows());
    _ASSERT(getRows() == mat.getRows());

    MatrixDT tmp(mat);
    for (unsigned row = 0; row < getRows(); ++row)
      for (unsigned col = 0; col < mat.getCols(); ++col)
        mat(row, col) = prod(*this, tmp, row, col, getCols());
  }
  void dotAdd(const MatrixDT &mat, MatrixDT &add) const {
    _ASSERT(getCols() == mat.getRows());
    _ASSERT(add.getRows() == getRows());
    _ASSERT(add.getCols() == mat.getCols());

    for (unsigned row = 0; row < getRows(); ++row)
      for (unsigned col = 0; col < mat.getCols(); ++col)
        add(row, col) += prod(*this, mat, row, col, getCols());
  }

  void set(unsigned rpos, unsigned cpos, const MatrixDT &mat) {
    for (unsigned row = 0; row < mat.getRows(); ++row)
      for (unsigned col = 0; col < mat.getCols(); ++col)
        get(rpos + row, cpos + col) = mat(row, col);
  }

  void calcStat(MatrixDT &res, enum StatMMAS<DT>::Kind kind, bool ByAbs, bool ByRows) const;
  MatrixDT calcStat(enum StatMMAS<DT>::Kind kind, bool ByAbs = false, bool ByRows = false) const {
    MatrixDT res;
    calcStat(res, kind, ByAbs, ByRows);
    return res;
  }

  MatrixDT buildDiagMat(int k = 0) const;
  void calcPseudoInverse();

  void resize(unsigned rows, unsigned cols, bool keep, bool zero = false, bool front = false);
  void trans(bool real = false) {
    _ASSERT(!real);  //!!!
    m_Trans = !m_Trans;
  }

  static MatrixDT getNormal(unsigned rows, unsigned cols, DT inMean = 0, DT inSigma = 1);

  MatrixDT calcFunc(DT(*pfunc)(DT)) const;

  void concatRows(const MatrixDT &mat);
  void concatCols(const MatrixDT &mat);

  MatrixDT repeat(unsigned nrows, unsigned ncols = 1) const;

  std::vector<DT> getAsVector() const;

  void addRand(DT ampl);

  MatrixDT &add(const MatrixDT &mat1, const MatrixDT &mat2);
  MatrixDT &operator += (const MatrixDT &mat) { return add(*this, mat); }
  MatrixDT operator + (const MatrixDT &mat) const { return MatrixDT().add(*this, mat); }

  MatrixDT &sub(const MatrixDT &mat1, const MatrixDT &mat2);
  MatrixDT &operator -= (const MatrixDT &mat) { return sub(*this, mat); }
  MatrixDT operator - (const MatrixDT &mat) const { return MatrixDT().sub(*this, mat); }

  MatrixDT &mul(const MatrixDT &mat, DT val);
  MatrixDT &operator *= (DT val) { return mul(*this, val); }
  MatrixDT operator * (DT val) const { return MatrixDT().mul(*this, val); }

  void print(const char *name = nullptr, char space = '\t') const;

private:
  void init(unsigned rows, unsigned cols) {
    m_Ptr = nullptr;
    m_BlkLen = cols;
    m_BlkQty = rows;
    m_Trans = false;
    m_Alien = false;
    m_Const = false;
  }

  const DT *getRowPtr(unsigned row) const {
    if (!m_Trans)
      return &m_Ptr[row * m_BlkLen];
    if (m_BlkQty == 1)
      return &m_Ptr[row];
    return nullptr;
  }
  const DT *getColPtr(unsigned col) const {
    if (m_Trans)
      return &m_Ptr[col * m_BlkLen];
    if (m_BlkQty == 1)
      return &m_Ptr[col];
    return nullptr;
  }
  DT *getRowPtr(unsigned row) { return const_cast<DT *>(const_cast<const MatrixDT *>(this)->getRowPtr(row)); }
  DT *getColPtr(unsigned col) { return const_cast<DT *>(const_cast<const MatrixDT *>(this)->getColPtr(col)); }

  const DT &getRC(unsigned i, unsigned j) const { return m_Ptr[i * m_BlkLen + j]; }
  const DT &getCR(unsigned i, unsigned j) const { return m_Ptr[j * m_BlkLen + i]; }
  DT &getRC(unsigned i, unsigned j) { return const_cast<DT &>(const_cast<const MatrixDT *>(this)->getRC(i, j)); }
  DT &getCR(unsigned i, unsigned j) { return const_cast<DT &>(const_cast<const MatrixDT *>(this)->getCR(i, j)); }

  static DT prod(const MatrixDT &left, const MatrixDT &right, unsigned row, unsigned col, unsigned mids) {
    DT val = 0;
    for (unsigned ind = 0; ind < mids; ++ind)
      val += left(row, ind) * right(ind, col);
    return val;
  }

  void free() {
    if (m_Ptr && !m_Alien)
      freeElms(getElms());
  }

  void changeMem(int change) {
    s_MemTotal += change;
    if (s_MemTotal > s_MemPeak)
      s_MemPeak = s_MemTotal;
  }
  void allocElms(unsigned elms, bool zero = false) {
    m_Ptr = (DT *)(!zero ? ::malloc(elms * sizeof(DT)) : calloc(elms, sizeof(DT)));
    _ASSERT(m_Ptr);
    changeMem(elms * sizeof(DT));
  }
  void reallocElms(unsigned old_elms, unsigned new_elms, bool zero = false) {
    _ASSERT(m_Ptr);
    m_Ptr = (DT *)(::realloc((void *)m_Ptr, new_elms * sizeof(DT)));
    _ASSERT(m_Ptr);
    if (zero && new_elms > old_elms)
      memset((void *)(m_Ptr + old_elms), '\0', (new_elms - old_elms) * sizeof(DT));
    changeMem(((int)new_elms - (int)old_elms) * sizeof(DT));
  }
  void freeElms(unsigned elms) {
    ::free(m_Ptr);
    changeMem(-(int)(elms * sizeof(DT)));
  }

public:
  static unsigned long s_MemTotal, s_MemPeak;

private:
  DT *m_Ptr;
  unsigned m_BlkLen, m_BlkQty;
  bool m_Trans;
  bool m_Alien;
  bool m_Const;
};

#include "Matrix_impl.hpp"

#include "Matrix_svd.hpp"

template<typename DT>
void MatrixDT<DT>::calcPseudoInverse() {
  MatrixDT d, vt;
  MatrixSVD<DT> svdSolver(*this, vt, d);
  vt.trans();
  d = d.buildDiagMat();

  MatrixDT dPlus(d.getCols(), d.getRows(), true); // transpose
  for (unsigned i = 0; i < d.getRows(); ++i)
    dPlus(i, i) = d(i, i).getInv();

  //vt.transpose().dot(dPlus).dot(u.transpose())
  vt.trans();
  vt = vt.dot(dPlus);
  trans();
  *this = vt.dot(*this);
}

template<typename DT>
unsigned long MatrixDT<DT>::s_MemTotal;
template<typename DT>
unsigned long MatrixDT<DT>::s_MemPeak;