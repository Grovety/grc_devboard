#include "Common.hpp"

static constexpr char TAG[] = "Common";

std::vector<float> saveToVect(const Matrix &mat) {
  std::vector<float> res;
  res.reserve(mat.getElms());
  for (unsigned col = 0; col < mat.numCols(); ++col)
    for (unsigned row = 0; row < mat.numRows(); ++row)
      res.push_back((float)mat(row, col));
  return res;
}

void printMatrix(const Matrix &mat, const char *name, const char sep) {
  if (name)
    printf("%s:\n", name);

  for (unsigned row = 0; row < mat.numRows(); ++row) {
    for (unsigned col = 0; col < mat.numCols(); ++col) {
      printf("% 4.4f", (float)mat(row, col));
      if (col + 1 < mat.numCols())
        printf("%c", sep);
    }
    printf("\n");
  }
}

bool walk(Matrix &mat, WalkFuncDyn func, void *data) {
  for (unsigned row = 0; row < mat.numRows(); ++row) {
    for (unsigned col = 0; col < mat.numCols(); ++col) {
      WALK_CMD cmd = (*func)(mat, row, col, data);
      if (cmd == WC_STOP)
        return false;
      if (cmd == WC_SKIP)
        break;
    }
  }
  return true;
}
