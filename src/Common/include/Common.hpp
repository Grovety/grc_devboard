#pragma once
#include "Types.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Log.hpp"

inline unsigned getTimeMS() { return xTaskGetTickCount() * portTICK_PERIOD_MS; }

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

/*!
  * \brief Convert matrix to vector.
  * \param mat Matrix.
  * \return Vector.
  */
std::vector<float> saveToVect(const Matrix &mat);
/*!
  * \brief Print matrix to STDOUT.
  * \param mat Matrix.
  * \param name Name.
  * \param sep Separator.
  */
void printMatrix(const Matrix &mat, const char *name,
                 const char sep = '\t');
/*!
  * \brief Convert signal data to Matrix.
  * \param data Signal data.
  * \param copy Whether to create copy.
  * \param take_ownership Whether to take ownership.
  * \param return Matrix.
  */
inline Matrix convert(const SignalData_t &data, bool copy, bool take_ownership) {
  Matrix ret((Matrix::ElmType *)data.buffer, data.num_rows, data.num_cols, copy, take_ownership, true);
  ret.trans();
  return ret;
}

enum WALK_CMD { WC_NEXT, WC_SKIP, WC_STOP };
typedef WALK_CMD (*WalkFuncDyn)(Matrix &mat, unsigned row, unsigned col,
                                void *data);
/*!
  * \brief Apply func on every element of matrix.
  * \param mat Matrix with signal.
  * \param func WALK_CMD function.
  * \param data User data.
  * \return Whether to stop.
  */
bool walk(Matrix &mat, WalkFuncDyn func, void *data = nullptr);