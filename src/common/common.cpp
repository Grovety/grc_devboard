#include "common.hpp"

constexpr char TAG[] = "common";

std::vector<float> saveToVect(const MatrixDyn &mat) {
  std::vector<float> res;
  res.reserve(mat.size());
  for (unsigned col = 0; col < mat.numCols(); ++col)
    for (unsigned row = 0; row < mat.numRows(); ++row)
      res.push_back((float)mat(row, col));
  return res;
}

void makeMatrix(MatrixDyn &mat, unsigned rows, unsigned cols,
                const float *ptr) {
  LOGD(TAG, "make matrix(%d, %d)", rows, cols);
  mat.resizeFast(rows, cols);
  for (unsigned col = 0; col < cols; ++col) {
    for (unsigned row = 0; row < rows; ++row) {
      mat(row, col) = ptr[col * rows + row];
    }
  }
};

void printMatrix(const MatrixDyn &mat, const char *name, const char space) {
  if (name)
    printf("%s:\n", name);

  for (unsigned row = 0; row < mat.numRows(); ++row) {
    for (unsigned col = 0; col < mat.numCols(); ++col) {
      printf("% 4.4f", mat(row, col));
      if (col + 1 < mat.numCols())
        printf("%c", space);
    }
    printf("\n");
  }
}
