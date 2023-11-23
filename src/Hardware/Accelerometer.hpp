#pragma once

#include "ISensor.hpp"

class Accelerometer : public ISensor {
public:
  enum : unsigned {
    Acl_x = 0,
    Acl_y,
    Acl_z,
    Vel_x,
    Vel_y,
    Vel_z,
    CompNum,
  };

  bool open() override final;
  bool close() override final;
  void getData(float *buf, unsigned len) override final;
};
