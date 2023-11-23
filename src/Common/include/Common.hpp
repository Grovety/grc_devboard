#pragma once
#include "Types.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Log.hpp"

inline void delayMS(unsigned time_ms) {
  vTaskDelay(time_ms / portTICK_PERIOD_MS);
}

inline unsigned getTimeMS() { return xTaskGetTickCount() * portTICK_PERIOD_MS; }

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

/*!
  * \brief Convert matrix to vector.
  * \param mat Matrix.
  * \return Vector.
  */
std::vector<float> saveToVect(const MatrixDyn &mat);
/*!
  * \brief Construct matrix from raw data.
  * \param rows Number of rows.
  * \param cols Number of columns.
  * \param ptr Buffer.
  */
void makeMatrix(MatrixDyn &mat, unsigned rows, unsigned cols, const float *ptr);
/*!
  * \brief Print matrix to STDOUT.
  * \param mat Matrix.
  * \param name Name.
  * \param sep Separator.
  */
void printMatrix(const MatrixDyn &mat, const char *name,
                 const char sep = '\t');
/*!
  * \brief Concatenate matrices by columns.
  * \param dst Destination.
  * \param src Source.
  */
void concat(MatrixDyn &dst, const MatrixDyn &src);

enum WALK_CMD { WC_NEXT, WC_SKIP, WC_STOP };
typedef WALK_CMD (*WalkFuncDyn)(MatrixDyn &mat, unsigned row, unsigned col,
                                void *data);
/*!
  * \brief Apply func on every element of matrix.
  * \param mat Matrix with signal.
  * \param func WALK_CMD function.
  * \param data User data.
  * \return Whether to stop.
  */
bool walk(MatrixDyn &mat, WalkFuncDyn func, void *data = nullptr);