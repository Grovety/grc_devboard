#pragma once
#include "custom_types.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "log.hpp"

inline void delayMS(unsigned time_ms) {
  vTaskDelay(time_ms / portTICK_PERIOD_MS);
}

inline unsigned getTimeMS() { return xTaskGetTickCount() * portTICK_PERIOD_MS; }

#ifndef _countof
#define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

std::vector<float> saveToVect(const MatrixDyn &mat);
void makeMatrix(MatrixDyn &mat, unsigned rows, unsigned cols, const float *ptr);
void printMatrix(const MatrixDyn &mat, const char *name,
                 const char space = '\t');
