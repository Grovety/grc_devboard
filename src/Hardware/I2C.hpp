#pragma once

#include <stdint.h>
#include <stddef.h>

class I2C {
private:
  const int sda_, scl_;
  bool isOpened_;
public:
  I2C(int sda, int scl) : sda_(sda), scl_(scl) { isOpened_ = false; }
  bool open();
  bool close();

  bool write(uint8_t dev_adr, const uint8_t *buf, size_t size);
  bool write_read(uint8_t dev_adr, const uint8_t* wbuf, size_t wsize, uint8_t* rbuf, size_t rsize);
};

extern I2C g_i2c;