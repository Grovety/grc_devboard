#include "GestureGrc.hpp"

static void smooth(Matrix &dst, const Matrix &src, unsigned avg_n,
                   unsigned comps, unsigned beg) {
  unsigned cols = src.numCols() / avg_n;
  dst = Matrix(comps, cols, true);
  for (unsigned dcol = 0, scol = 0; dcol < cols; ++dcol) {
    for (unsigned c = 0; c < avg_n; ++c)
      for (unsigned r = 0; r < comps; ++r)
        dst(r, dcol) += src(beg + r, scol + c);

    scol += avg_n;

    for (unsigned r = 0; r < comps; ++r)
      dst(r, dcol) /= avg_n;
  }
}

static Matrix slide(const Matrix &src, int avg_n) {
  unsigned rows = src.getRows();
  int cols = src.getCols();
  Matrix res = Matrix(rows, cols - 2 * avg_n, true);
  for (unsigned row = 0; row < rows; ++row) {
    for (int col = avg_n; col < cols - avg_n; ++col) {
      for (int c = -avg_n; c <= avg_n; ++c)
        res(row, col - avg_n) += src(row, col + c);

      res(row, col - avg_n) /= (2 * avg_n);
    }
  }
  return res;
}

static Matrix fixLen(const Matrix &src, unsigned Len) {
  unsigned rows = src.getRows();
  unsigned cols = src.getCols();
  float fact = (float)cols / Len;

  Matrix res(rows, Len);
  for (unsigned dcol = 0; dcol < Len; ++dcol) {
    float x = fact * dcol;
    unsigned scol = (unsigned)x;
    float dlt = x - scol;
    for (unsigned row = 0; row < rows; ++row) {
      res(row, dcol) = src(row, scol);
      if (scol + 1 < cols)
        res(row, dcol) += dlt * (src(row, scol + 1) - src(row, scol));
    }
  }
  return res;
}

int getSignalBeg(const Matrix &src, unsigned percent) {
  auto getAcl = [](const Matrix &mat, unsigned col) {
    return fabs(mat(3, col)) + fabs(mat(4, col)) + fabs(mat(5, col));
  };

  unsigned cols = src.getCols();
  float MMin = getAcl(src, 0) * (1 - percent / 100.);
  float MMax = getAcl(src, 0) * (1 + percent / 100.);
  for (unsigned col = 0; col < cols; ++col) {
    float acl = getAcl(src, col);
    if (acl < MMin || acl > MMax) {
      col = (col > 40 ? col - 40 : 0);
      return col;
    }
  }
  return -1;
}

static Matrix fixBeg(const Matrix &src, unsigned percent) {
  int col = getSignalBeg(src, percent);
  if (col >= 0) {
    return Matrix(src, Matrix::Rect(0, col), true);
  }

  return src;
}

int GestureGrc::train(Matrix &signal, int category) {
  preprocess(signal);
  const auto vec = saveToVect(signal);
  return grc_.train(vec.size(), vec.data(), category);
}

int GestureGrc::inference(Matrix &signal) {
  preprocess(signal);
  const auto vec = saveToVect(signal);
  const auto category = grc_.inference(vec.size(), vec.data());
  return category;
}

void GestureGrc::preprocess(Matrix &signal) const {
  signal = slide(signal, 20);

  signal = fixBeg(signal, 20);

  signal = fixLen(signal, 1000);

  Matrix pr_sig;
  smooth(pr_sig, signal, 10, SIGNAL_COMPS_NUM, 0);
  signal = pr_sig;

  signal = signal.repeat(1, 3);

  for (unsigned row = 0; row < signal.getRows() / 2; ++row) {
    for (unsigned col = 0; col < signal.getCols(); ++col) {
      Matrix::ElmType t = signal(row, col);
      signal(row, col) = signal(row + signal.getRows() / 2, col);
      signal(row + signal.getRows() / 2, col) = t;
    }
  }
}