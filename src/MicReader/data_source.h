#ifndef _DATA_SOURCE_H_
#define _DATA_SOURCE_H_

#include <stddef.h>

class Preprocessor {
public:
  virtual size_t Apply(const void* src, const size_t num_samples, void* dst, const int n_frames) = 0;
  virtual int Errors() {return 0;};
};

class DataSource {
public:
  virtual void Setup() {};
  virtual void Close() {};
  virtual size_t Collect(Preprocessor* pp, void** dst, int flag) = 0;
  virtual int Errors() {return 0;};
};

#endif // _DATA_SOURCE_H_