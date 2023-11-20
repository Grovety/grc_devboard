// tvm target: c -keys=riscv_cpu 
#define TVM_EXPORTS
#include "tvm/runtime/c_runtime_api.h"
#include "tvm/runtime/c_backend_api.h"
#include <math.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_avg_pool2d_cast(int32_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_24_var, uint8_t* global_workspace_25_var) {
  void* pool_sum_let = (&(global_workspace_25_var[32000]));
  for (int32_t ax3_init = 0; ax3_init < 64; ++ax3_init) {
    ((int32_t*)pool_sum_let)[ax3_init] = 0;
  }
  for (int32_t rv0_rv1_fused = 0; rv0_rv1_fused < 125; ++rv0_rv1_fused) {
    for (int32_t ax3 = 0; ax3 < 64; ++ax3) {
      ((int32_t*)pool_sum_let)[ax3] = (((int32_t*)pool_sum_let)[ax3] + p0[((rv0_rv1_fused * 64) + ax3)]);
    }
  }
  for (int32_t ax3_1 = 0; ax3_1 < 64; ++ax3_1) {
    T_cast[ax3_1] = ((int8_t)(((int32_t*)pool_sum_let)[ax3_1] / 125));
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip(int8_t* p0, int32_t* compute, uint8_t* global_const_workspace_22_var, uint8_t* global_workspace_23_var) {
  void* fused_nn_conv2d_subtract_add_constant_26_let = (&(global_const_workspace_22_var[26176]));
  void* fused_nn_conv2d_subtract_add_constant_25_let = (&(global_const_workspace_22_var[26432]));
  void* fused_nn_conv2d_subtract_add_constant_24_let = (&(global_const_workspace_22_var[26688]));
  void* fused_nn_conv2d_constant_8_let = (&(global_const_workspace_22_var[31296]));
  void* fused_nn_conv2d_subtract_constant_8_let = (&(global_const_workspace_22_var[22080]));
  void* fused_constant_8_let = (&(global_const_workspace_22_var[0]));
  void* pad_temp_let = (&(global_workspace_23_var[32000]));
  for (int32_t i0_i1_fused = 0; i0_i1_fused < 25; ++i0_i1_fused) {
    for (int32_t i2 = 0; i2 < 5; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i0_i1_fused * 320) + (i2 * 64)) + i3);
        ((int8_t*)pad_temp_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t i0_i1_fused_i2_fused = 0; i0_i1_fused_i2_fused < 125; ++i0_i1_fused_i2_fused) {
    void* conv2d_nhwc_let = (&(global_workspace_23_var[40000]));
    for (int32_t ff = 0; ff < 64; ++ff) {
      ((int32_t*)conv2d_nhwc_let)[ff] = 0;
      for (int32_t rc = 0; rc < 64; ++rc) {
        ((int32_t*)conv2d_nhwc_let)[ff] = (((int32_t*)conv2d_nhwc_let)[ff] + (((int32_t)((int8_t*)pad_temp_let)[((i0_i1_fused_i2_fused * 64) + rc)]) * ((int32_t)((int8_t*)fused_constant_8_let)[((rc * 64) + ff)])));
      }
    }
    for (int32_t i3_inner = 0; i3_inner < 64; ++i3_inner) {
      int32_t v_ = ((int32_t)(((((int64_t)((((int32_t*)conv2d_nhwc_let)[i3_inner] + ((int32_t*)fused_nn_conv2d_subtract_constant_8_let)[i3_inner]) - ((int32_t*)fused_nn_conv2d_constant_8_let)[i3_inner])) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_24_let)[i3_inner])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_26_let)[i3_inner] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_26_let)[i3_inner] + 31)))) - 128;
      int32_t v__1 = (v_) < (127) ? (v_) : (127);
      compute[((i0_i1_fused_i2_fused * 64) + i3_inner)] = ((v__1) > (-128) ? (v__1) : (-128));
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_4_var, uint8_t* global_workspace_5_var) {
  void* fused_nn_conv2d_subtract_add_constant_2_let = (&(global_const_workspace_4_var[27968]));
  void* fused_nn_conv2d_subtract_add_constant_1_let = (&(global_const_workspace_4_var[30784]));
  void* fused_nn_conv2d_subtract_add_constant_let = (&(global_const_workspace_4_var[31040]));
  void* fused_nn_conv2d_constant_let = (&(global_const_workspace_4_var[33344]));
  void* fused_nn_conv2d_subtract_constant_let = (&(global_const_workspace_4_var[24128]));
  void* fused_constant_let = (&(global_const_workspace_4_var[16384]));
  void* pad_temp_let = (&(global_workspace_5_var[20096]));
  for (int32_t i0_i1_fused = 0; i0_i1_fused < 58; ++i0_i1_fused) {
    for (int32_t i2 = 0; i2 < 12; ++i2) {
      int32_t cse_var_1 = ((i0_i1_fused * 12) + i2);
      ((int8_t*)pad_temp_let)[cse_var_1] = p0[cse_var_1];
    }
  }
  for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 125; ++ax0_ax1_fused_ax2_fused) {
    void* conv2d_nhwc_let = (&(global_workspace_5_var[20800]));
    for (int32_t ff = 0; ff < 64; ++ff) {
      ((int32_t*)conv2d_nhwc_let)[ff] = 0;
      for (int32_t ry = 0; ry < 10; ++ry) {
        for (int32_t rx = 0; rx < 4; ++rx) {
          ((int32_t*)conv2d_nhwc_let)[ff] = (((int32_t*)conv2d_nhwc_let)[ff] + (((int32_t)((int8_t*)pad_temp_let)[(((((ax0_ax1_fused_ax2_fused / 5) * 24) + (ry * 12)) + ((ax0_ax1_fused_ax2_fused % 5) * 2)) + rx)]) * ((int32_t)((int8_t*)fused_constant_let)[(((ry * 256) + (rx * 64)) + ff)])));
        }
      }
    }
    for (int32_t ax3_inner = 0; ax3_inner < 64; ++ax3_inner) {
      int32_t v_ = ((int32_t)(((((int64_t)((((int32_t*)conv2d_nhwc_let)[ax3_inner] + ((int32_t*)fused_nn_conv2d_subtract_constant_let)[ax3_inner]) - ((int32_t*)fused_nn_conv2d_constant_let)[ax3_inner])) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_let)[ax3_inner])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_2_let)[ax3_inner] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_2_let)[ax3_inner] + 31)))) - 128;
      int32_t v__1 = (v_) < (127) ? (v_) : (127);
      T_cast[((ax0_ax1_fused_ax2_fused * 64) + ax3_inner)] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_1(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_8_var, uint8_t* global_workspace_9_var) {
  void* fused_nn_conv2d_subtract_add_constant_5_let = (&(global_const_workspace_8_var[25408]));
  void* fused_nn_conv2d_subtract_add_constant_4_let = (&(global_const_workspace_8_var[25664]));
  void* fused_nn_conv2d_subtract_add_constant_3_let = (&(global_const_workspace_8_var[25920]));
  void* fused_nn_conv2d_subtract_constant_1_let = (&(global_const_workspace_8_var[23872]));
  void* fused_nn_conv2d_constant_1_let = (&(global_const_workspace_8_var[33088]));
  void* fused_constant_1_let = (&(global_const_workspace_8_var[21504]));
  void* PaddedInput_let = (&(global_workspace_9_var[32000]));
  void* DepthwiseConv2d_let = (&(global_workspace_9_var[0]));
  for (int32_t i1 = 0; i1 < 27; ++i1) {
    for (int32_t i2 = 0; i2 < 7; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i1 * 448) + (i2 * 64)) + i3);
        ((int8_t*)PaddedInput_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t i = 0; i < 25; ++i) {
    for (int32_t j = 0; j < 5; ++j) {
      for (int32_t c = 0; c < 64; ++c) {
        ((int32_t*)DepthwiseConv2d_let)[(((i * 320) + (j * 64)) + c)] = 0;
        for (int32_t di = 0; di < 3; ++di) {
          for (int32_t dj = 0; dj < 3; ++dj) {
            int32_t cse_var_4 = (j * 64);
            int32_t cse_var_3 = (dj * 64);
            int32_t cse_var_2 = (((i * 320) + cse_var_4) + c);
            ((int32_t*)DepthwiseConv2d_let)[cse_var_2] = (((int32_t*)DepthwiseConv2d_let)[cse_var_2] + (((int32_t)((int8_t*)PaddedInput_let)[(((((i * 448) + (di * 448)) + cse_var_4) + cse_var_3) + c)]) * ((int32_t)((int8_t*)fused_constant_1_let)[(((di * 192) + cse_var_3) + c)])));
          }
        }
      }
    }
  }
  for (int32_t ax1 = 0; ax1 < 25; ++ax1) {
    for (int32_t ax2 = 0; ax2 < 5; ++ax2) {
      for (int32_t ax3 = 0; ax3 < 64; ++ax3) {
        int32_t cse_var_5 = (((ax1 * 320) + (ax2 * 64)) + ax3);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_5] = (((int32_t*)DepthwiseConv2d_let)[cse_var_5] - ((int32_t*)fused_nn_conv2d_constant_1_let)[ax3]);
      }
    }
  }
  for (int32_t ax1_1 = 0; ax1_1 < 25; ++ax1_1) {
    for (int32_t ax2_1 = 0; ax2_1 < 5; ++ax2_1) {
      for (int32_t ax3_1 = 0; ax3_1 < 64; ++ax3_1) {
        int32_t cse_var_6 = (((ax1_1 * 320) + (ax2_1 * 64)) + ax3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_6] = (((int32_t*)DepthwiseConv2d_let)[cse_var_6] + ((int32_t*)fused_nn_conv2d_subtract_constant_1_let)[ax3_1]);
      }
    }
  }
  for (int32_t i1_1 = 0; i1_1 < 25; ++i1_1) {
    for (int32_t i2_1 = 0; i2_1 < 5; ++i2_1) {
      for (int32_t i3_1 = 0; i3_1 < 64; ++i3_1) {
        int32_t cse_var_7 = (((i1_1 * 320) + (i2_1 * 64)) + i3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_7] = ((int32_t)(((((int64_t)((int32_t*)DepthwiseConv2d_let)[cse_var_7]) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_3_let)[i3_1])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_5_let)[i3_1] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_5_let)[i3_1] + 31))));
      }
    }
  }
  for (int32_t ax1_2 = 0; ax1_2 < 25; ++ax1_2) {
    for (int32_t ax2_2 = 0; ax2_2 < 5; ++ax2_2) {
      for (int32_t ax3_2 = 0; ax3_2 < 64; ++ax3_2) {
        int32_t cse_var_8 = (((ax1_2 * 320) + (ax2_2 * 64)) + ax3_2);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_8] = (((int32_t*)DepthwiseConv2d_let)[cse_var_8] - 128);
      }
    }
  }
  for (int32_t i1_2 = 0; i1_2 < 25; ++i1_2) {
    for (int32_t i2_2 = 0; i2_2 < 5; ++i2_2) {
      for (int32_t i3_2 = 0; i3_2 < 64; ++i3_2) {
        int32_t cse_var_9 = (((i1_2 * 320) + (i2_2 * 64)) + i3_2);
        int32_t v_ = ((int32_t*)DepthwiseConv2d_let)[cse_var_9];
        int32_t v__1 = (v_) < (127) ? (v_) : (127);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_9] = ((v__1) > (-128) ? (v__1) : (-128));
      }
    }
  }
  for (int32_t ax1_3 = 0; ax1_3 < 25; ++ax1_3) {
    for (int32_t ax2_3 = 0; ax2_3 < 5; ++ax2_3) {
      for (int32_t ax3_3 = 0; ax3_3 < 64; ++ax3_3) {
        int32_t cse_var_10 = (((ax1_3 * 320) + (ax2_3 * 64)) + ax3_3);
        T_cast[cse_var_10] = ((int8_t)((int32_t*)DepthwiseConv2d_let)[cse_var_10]);
      }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_2(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_10_var, uint8_t* global_workspace_11_var) {
  void* fused_nn_conv2d_subtract_add_constant_8_let = (&(global_const_workspace_10_var[24640]));
  void* fused_nn_conv2d_subtract_add_constant_7_let = (&(global_const_workspace_10_var[24896]));
  void* fused_nn_conv2d_subtract_add_constant_6_let = (&(global_const_workspace_10_var[25152]));
  void* fused_nn_conv2d_constant_2_let = (&(global_const_workspace_10_var[32832]));
  void* fused_nn_conv2d_subtract_constant_2_let = (&(global_const_workspace_10_var[23616]));
  void* fused_constant_2_let = (&(global_const_workspace_10_var[12288]));
  void* pad_temp_let = (&(global_workspace_11_var[20096]));
  for (int32_t i0_i1_fused = 0; i0_i1_fused < 25; ++i0_i1_fused) {
    for (int32_t i2 = 0; i2 < 5; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i0_i1_fused * 320) + (i2 * 64)) + i3);
        ((int8_t*)pad_temp_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 125; ++ax0_ax1_fused_ax2_fused) {
    void* conv2d_nhwc_let = (&(global_workspace_11_var[28096]));
    for (int32_t ff = 0; ff < 64; ++ff) {
      ((int32_t*)conv2d_nhwc_let)[ff] = 0;
      for (int32_t rc = 0; rc < 64; ++rc) {
        ((int32_t*)conv2d_nhwc_let)[ff] = (((int32_t*)conv2d_nhwc_let)[ff] + (((int32_t)((int8_t*)pad_temp_let)[((ax0_ax1_fused_ax2_fused * 64) + rc)]) * ((int32_t)((int8_t*)fused_constant_2_let)[((rc * 64) + ff)])));
      }
    }
    for (int32_t ax3_inner = 0; ax3_inner < 64; ++ax3_inner) {
      int32_t v_ = ((int32_t)(((((int64_t)((((int32_t*)conv2d_nhwc_let)[ax3_inner] + ((int32_t*)fused_nn_conv2d_subtract_constant_2_let)[ax3_inner]) - ((int32_t*)fused_nn_conv2d_constant_2_let)[ax3_inner])) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_6_let)[ax3_inner])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_8_let)[ax3_inner] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_8_let)[ax3_inner] + 31)))) - 128;
      int32_t v__1 = (v_) < (127) ? (v_) : (127);
      T_cast[((ax0_ax1_fused_ax2_fused * 64) + ax3_inner)] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_3(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_12_var, uint8_t* global_workspace_13_var) {
  void* fused_nn_conv2d_subtract_add_constant_11_let = (&(global_const_workspace_12_var[30272]));
  void* fused_nn_conv2d_subtract_add_constant_10_let = (&(global_const_workspace_12_var[30528]));
  void* fused_nn_conv2d_subtract_add_constant_9_let = (&(global_const_workspace_12_var[24384]));
  void* fused_nn_conv2d_subtract_constant_3_let = (&(global_const_workspace_12_var[23360]));
  void* fused_nn_conv2d_constant_3_let = (&(global_const_workspace_12_var[32576]));
  void* fused_constant_3_let = (&(global_const_workspace_12_var[20928]));
  void* PaddedInput_let = (&(global_workspace_13_var[32000]));
  void* DepthwiseConv2d_let = (&(global_workspace_13_var[0]));
  for (int32_t i1 = 0; i1 < 27; ++i1) {
    for (int32_t i2 = 0; i2 < 7; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i1 * 448) + (i2 * 64)) + i3);
        ((int8_t*)PaddedInput_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t i = 0; i < 25; ++i) {
    for (int32_t j = 0; j < 5; ++j) {
      for (int32_t c = 0; c < 64; ++c) {
        ((int32_t*)DepthwiseConv2d_let)[(((i * 320) + (j * 64)) + c)] = 0;
        for (int32_t di = 0; di < 3; ++di) {
          for (int32_t dj = 0; dj < 3; ++dj) {
            int32_t cse_var_4 = (j * 64);
            int32_t cse_var_3 = (dj * 64);
            int32_t cse_var_2 = (((i * 320) + cse_var_4) + c);
            ((int32_t*)DepthwiseConv2d_let)[cse_var_2] = (((int32_t*)DepthwiseConv2d_let)[cse_var_2] + (((int32_t)((int8_t*)PaddedInput_let)[(((((i * 448) + (di * 448)) + cse_var_4) + cse_var_3) + c)]) * ((int32_t)((int8_t*)fused_constant_3_let)[(((di * 192) + cse_var_3) + c)])));
          }
        }
      }
    }
  }
  for (int32_t ax1 = 0; ax1 < 25; ++ax1) {
    for (int32_t ax2 = 0; ax2 < 5; ++ax2) {
      for (int32_t ax3 = 0; ax3 < 64; ++ax3) {
        int32_t cse_var_5 = (((ax1 * 320) + (ax2 * 64)) + ax3);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_5] = (((int32_t*)DepthwiseConv2d_let)[cse_var_5] - ((int32_t*)fused_nn_conv2d_constant_3_let)[ax3]);
      }
    }
  }
  for (int32_t ax1_1 = 0; ax1_1 < 25; ++ax1_1) {
    for (int32_t ax2_1 = 0; ax2_1 < 5; ++ax2_1) {
      for (int32_t ax3_1 = 0; ax3_1 < 64; ++ax3_1) {
        int32_t cse_var_6 = (((ax1_1 * 320) + (ax2_1 * 64)) + ax3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_6] = (((int32_t*)DepthwiseConv2d_let)[cse_var_6] + ((int32_t*)fused_nn_conv2d_subtract_constant_3_let)[ax3_1]);
      }
    }
  }
  for (int32_t i1_1 = 0; i1_1 < 25; ++i1_1) {
    for (int32_t i2_1 = 0; i2_1 < 5; ++i2_1) {
      for (int32_t i3_1 = 0; i3_1 < 64; ++i3_1) {
        int32_t cse_var_7 = (((i1_1 * 320) + (i2_1 * 64)) + i3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_7] = ((int32_t)(((((int64_t)((int32_t*)DepthwiseConv2d_let)[cse_var_7]) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_9_let)[i3_1])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_11_let)[i3_1] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_11_let)[i3_1] + 31))));
      }
    }
  }
  for (int32_t ax1_2 = 0; ax1_2 < 25; ++ax1_2) {
    for (int32_t ax2_2 = 0; ax2_2 < 5; ++ax2_2) {
      for (int32_t ax3_2 = 0; ax3_2 < 64; ++ax3_2) {
        int32_t cse_var_8 = (((ax1_2 * 320) + (ax2_2 * 64)) + ax3_2);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_8] = (((int32_t*)DepthwiseConv2d_let)[cse_var_8] - 128);
      }
    }
  }
  for (int32_t i1_2 = 0; i1_2 < 25; ++i1_2) {
    for (int32_t i2_2 = 0; i2_2 < 5; ++i2_2) {
      for (int32_t i3_2 = 0; i3_2 < 64; ++i3_2) {
        int32_t cse_var_9 = (((i1_2 * 320) + (i2_2 * 64)) + i3_2);
        int32_t v_ = ((int32_t*)DepthwiseConv2d_let)[cse_var_9];
        int32_t v__1 = (v_) < (127) ? (v_) : (127);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_9] = ((v__1) > (-128) ? (v__1) : (-128));
      }
    }
  }
  for (int32_t ax1_3 = 0; ax1_3 < 25; ++ax1_3) {
    for (int32_t ax2_3 = 0; ax2_3 < 5; ++ax2_3) {
      for (int32_t ax3_3 = 0; ax3_3 < 64; ++ax3_3) {
        int32_t cse_var_10 = (((ax1_3 * 320) + (ax2_3 * 64)) + ax3_3);
        T_cast[cse_var_10] = ((int8_t)((int32_t*)DepthwiseConv2d_let)[cse_var_10]);
      }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_4(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_14_var, uint8_t* global_workspace_15_var) {
  void* fused_nn_conv2d_subtract_add_constant_14_let = (&(global_const_workspace_14_var[29504]));
  void* fused_nn_conv2d_subtract_add_constant_13_let = (&(global_const_workspace_14_var[29760]));
  void* fused_nn_conv2d_subtract_add_constant_12_let = (&(global_const_workspace_14_var[30016]));
  void* fused_nn_conv2d_constant_4_let = (&(global_const_workspace_14_var[32320]));
  void* fused_nn_conv2d_subtract_constant_4_let = (&(global_const_workspace_14_var[23104]));
  void* fused_constant_4_let = (&(global_const_workspace_14_var[8192]));
  void* pad_temp_let = (&(global_workspace_15_var[20096]));
  for (int32_t i0_i1_fused = 0; i0_i1_fused < 25; ++i0_i1_fused) {
    for (int32_t i2 = 0; i2 < 5; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i0_i1_fused * 320) + (i2 * 64)) + i3);
        ((int8_t*)pad_temp_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 125; ++ax0_ax1_fused_ax2_fused) {
    void* conv2d_nhwc_let = (&(global_workspace_15_var[28096]));
    for (int32_t ff = 0; ff < 64; ++ff) {
      ((int32_t*)conv2d_nhwc_let)[ff] = 0;
      for (int32_t rc = 0; rc < 64; ++rc) {
        ((int32_t*)conv2d_nhwc_let)[ff] = (((int32_t*)conv2d_nhwc_let)[ff] + (((int32_t)((int8_t*)pad_temp_let)[((ax0_ax1_fused_ax2_fused * 64) + rc)]) * ((int32_t)((int8_t*)fused_constant_4_let)[((rc * 64) + ff)])));
      }
    }
    for (int32_t ax3_inner = 0; ax3_inner < 64; ++ax3_inner) {
      int32_t v_ = ((int32_t)(((((int64_t)((((int32_t*)conv2d_nhwc_let)[ax3_inner] + ((int32_t*)fused_nn_conv2d_subtract_constant_4_let)[ax3_inner]) - ((int32_t*)fused_nn_conv2d_constant_4_let)[ax3_inner])) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_12_let)[ax3_inner])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_14_let)[ax3_inner] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_14_let)[ax3_inner] + 31)))) - 128;
      int32_t v__1 = (v_) < (127) ? (v_) : (127);
      T_cast[((ax0_ax1_fused_ax2_fused * 64) + ax3_inner)] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_5(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_16_var, uint8_t* global_workspace_17_var) {
  void* fused_nn_conv2d_subtract_add_constant_17_let = (&(global_const_workspace_16_var[28736]));
  void* fused_nn_conv2d_subtract_add_constant_16_let = (&(global_const_workspace_16_var[28992]));
  void* fused_nn_conv2d_subtract_add_constant_15_let = (&(global_const_workspace_16_var[29248]));
  void* fused_nn_conv2d_subtract_constant_5_let = (&(global_const_workspace_16_var[22848]));
  void* fused_nn_conv2d_constant_5_let = (&(global_const_workspace_16_var[32064]));
  void* fused_constant_5_let = (&(global_const_workspace_16_var[20352]));
  void* PaddedInput_let = (&(global_workspace_17_var[32000]));
  void* DepthwiseConv2d_let = (&(global_workspace_17_var[0]));
  for (int32_t i1 = 0; i1 < 27; ++i1) {
    for (int32_t i2 = 0; i2 < 7; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i1 * 448) + (i2 * 64)) + i3);
        ((int8_t*)PaddedInput_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t i = 0; i < 25; ++i) {
    for (int32_t j = 0; j < 5; ++j) {
      for (int32_t c = 0; c < 64; ++c) {
        ((int32_t*)DepthwiseConv2d_let)[(((i * 320) + (j * 64)) + c)] = 0;
        for (int32_t di = 0; di < 3; ++di) {
          for (int32_t dj = 0; dj < 3; ++dj) {
            int32_t cse_var_4 = (j * 64);
            int32_t cse_var_3 = (dj * 64);
            int32_t cse_var_2 = (((i * 320) + cse_var_4) + c);
            ((int32_t*)DepthwiseConv2d_let)[cse_var_2] = (((int32_t*)DepthwiseConv2d_let)[cse_var_2] + (((int32_t)((int8_t*)PaddedInput_let)[(((((i * 448) + (di * 448)) + cse_var_4) + cse_var_3) + c)]) * ((int32_t)((int8_t*)fused_constant_5_let)[(((di * 192) + cse_var_3) + c)])));
          }
        }
      }
    }
  }
  for (int32_t ax1 = 0; ax1 < 25; ++ax1) {
    for (int32_t ax2 = 0; ax2 < 5; ++ax2) {
      for (int32_t ax3 = 0; ax3 < 64; ++ax3) {
        int32_t cse_var_5 = (((ax1 * 320) + (ax2 * 64)) + ax3);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_5] = (((int32_t*)DepthwiseConv2d_let)[cse_var_5] - ((int32_t*)fused_nn_conv2d_constant_5_let)[ax3]);
      }
    }
  }
  for (int32_t ax1_1 = 0; ax1_1 < 25; ++ax1_1) {
    for (int32_t ax2_1 = 0; ax2_1 < 5; ++ax2_1) {
      for (int32_t ax3_1 = 0; ax3_1 < 64; ++ax3_1) {
        int32_t cse_var_6 = (((ax1_1 * 320) + (ax2_1 * 64)) + ax3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_6] = (((int32_t*)DepthwiseConv2d_let)[cse_var_6] + ((int32_t*)fused_nn_conv2d_subtract_constant_5_let)[ax3_1]);
      }
    }
  }
  for (int32_t i1_1 = 0; i1_1 < 25; ++i1_1) {
    for (int32_t i2_1 = 0; i2_1 < 5; ++i2_1) {
      for (int32_t i3_1 = 0; i3_1 < 64; ++i3_1) {
        int32_t cse_var_7 = (((i1_1 * 320) + (i2_1 * 64)) + i3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_7] = ((int32_t)(((((int64_t)((int32_t*)DepthwiseConv2d_let)[cse_var_7]) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_15_let)[i3_1])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_17_let)[i3_1] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_17_let)[i3_1] + 31))));
      }
    }
  }
  for (int32_t ax1_2 = 0; ax1_2 < 25; ++ax1_2) {
    for (int32_t ax2_2 = 0; ax2_2 < 5; ++ax2_2) {
      for (int32_t ax3_2 = 0; ax3_2 < 64; ++ax3_2) {
        int32_t cse_var_8 = (((ax1_2 * 320) + (ax2_2 * 64)) + ax3_2);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_8] = (((int32_t*)DepthwiseConv2d_let)[cse_var_8] - 128);
      }
    }
  }
  for (int32_t i1_2 = 0; i1_2 < 25; ++i1_2) {
    for (int32_t i2_2 = 0; i2_2 < 5; ++i2_2) {
      for (int32_t i3_2 = 0; i3_2 < 64; ++i3_2) {
        int32_t cse_var_9 = (((i1_2 * 320) + (i2_2 * 64)) + i3_2);
        int32_t v_ = ((int32_t*)DepthwiseConv2d_let)[cse_var_9];
        int32_t v__1 = (v_) < (127) ? (v_) : (127);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_9] = ((v__1) > (-128) ? (v__1) : (-128));
      }
    }
  }
  for (int32_t ax1_3 = 0; ax1_3 < 25; ++ax1_3) {
    for (int32_t ax2_3 = 0; ax2_3 < 5; ++ax2_3) {
      for (int32_t ax3_3 = 0; ax3_3 < 64; ++ax3_3) {
        int32_t cse_var_10 = (((ax1_3 * 320) + (ax2_3 * 64)) + ax3_3);
        T_cast[cse_var_10] = ((int8_t)((int32_t*)DepthwiseConv2d_let)[cse_var_10]);
      }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_6(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_18_var, uint8_t* global_workspace_19_var) {
  void* fused_nn_conv2d_subtract_add_constant_20_let = (&(global_const_workspace_18_var[27712]));
  void* fused_nn_conv2d_subtract_add_constant_19_let = (&(global_const_workspace_18_var[28224]));
  void* fused_nn_conv2d_subtract_add_constant_18_let = (&(global_const_workspace_18_var[28480]));
  void* fused_nn_conv2d_constant_6_let = (&(global_const_workspace_18_var[31808]));
  void* fused_nn_conv2d_subtract_constant_6_let = (&(global_const_workspace_18_var[22592]));
  void* fused_constant_6_let = (&(global_const_workspace_18_var[4096]));
  void* pad_temp_let = (&(global_workspace_19_var[20096]));
  for (int32_t i0_i1_fused = 0; i0_i1_fused < 25; ++i0_i1_fused) {
    for (int32_t i2 = 0; i2 < 5; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i0_i1_fused * 320) + (i2 * 64)) + i3);
        ((int8_t*)pad_temp_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 125; ++ax0_ax1_fused_ax2_fused) {
    void* conv2d_nhwc_let = (&(global_workspace_19_var[28096]));
    for (int32_t ff = 0; ff < 64; ++ff) {
      ((int32_t*)conv2d_nhwc_let)[ff] = 0;
      for (int32_t rc = 0; rc < 64; ++rc) {
        ((int32_t*)conv2d_nhwc_let)[ff] = (((int32_t*)conv2d_nhwc_let)[ff] + (((int32_t)((int8_t*)pad_temp_let)[((ax0_ax1_fused_ax2_fused * 64) + rc)]) * ((int32_t)((int8_t*)fused_constant_6_let)[((rc * 64) + ff)])));
      }
    }
    for (int32_t ax3_inner = 0; ax3_inner < 64; ++ax3_inner) {
      int32_t v_ = ((int32_t)(((((int64_t)((((int32_t*)conv2d_nhwc_let)[ax3_inner] + ((int32_t*)fused_nn_conv2d_subtract_constant_6_let)[ax3_inner]) - ((int32_t*)fused_nn_conv2d_constant_6_let)[ax3_inner])) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_18_let)[ax3_inner])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_20_let)[ax3_inner] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_20_let)[ax3_inner] + 31)))) - 128;
      int32_t v__1 = (v_) < (127) ? (v_) : (127);
      T_cast[((ax0_ax1_fused_ax2_fused * 64) + ax3_inner)] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_7(int8_t* p0, int8_t* T_cast, uint8_t* global_const_workspace_20_var, uint8_t* global_workspace_21_var) {
  void* fused_nn_conv2d_subtract_add_constant_23_let = (&(global_const_workspace_20_var[26944]));
  void* fused_nn_conv2d_subtract_add_constant_22_let = (&(global_const_workspace_20_var[27200]));
  void* fused_nn_conv2d_subtract_add_constant_21_let = (&(global_const_workspace_20_var[27456]));
  void* fused_nn_conv2d_subtract_constant_7_let = (&(global_const_workspace_20_var[22336]));
  void* fused_nn_conv2d_constant_7_let = (&(global_const_workspace_20_var[31552]));
  void* fused_constant_7_let = (&(global_const_workspace_20_var[19776]));
  void* PaddedInput_let = (&(global_workspace_21_var[32000]));
  void* DepthwiseConv2d_let = (&(global_workspace_21_var[0]));
  for (int32_t i1 = 0; i1 < 27; ++i1) {
    for (int32_t i2 = 0; i2 < 7; ++i2) {
      for (int32_t i3 = 0; i3 < 64; ++i3) {
        int32_t cse_var_1 = (((i1 * 448) + (i2 * 64)) + i3);
        ((int8_t*)PaddedInput_let)[cse_var_1] = p0[cse_var_1];
      }
    }
  }
  for (int32_t i = 0; i < 25; ++i) {
    for (int32_t j = 0; j < 5; ++j) {
      for (int32_t c = 0; c < 64; ++c) {
        ((int32_t*)DepthwiseConv2d_let)[(((i * 320) + (j * 64)) + c)] = 0;
        for (int32_t di = 0; di < 3; ++di) {
          for (int32_t dj = 0; dj < 3; ++dj) {
            int32_t cse_var_4 = (j * 64);
            int32_t cse_var_3 = (dj * 64);
            int32_t cse_var_2 = (((i * 320) + cse_var_4) + c);
            ((int32_t*)DepthwiseConv2d_let)[cse_var_2] = (((int32_t*)DepthwiseConv2d_let)[cse_var_2] + (((int32_t)((int8_t*)PaddedInput_let)[(((((i * 448) + (di * 448)) + cse_var_4) + cse_var_3) + c)]) * ((int32_t)((int8_t*)fused_constant_7_let)[(((di * 192) + cse_var_3) + c)])));
          }
        }
      }
    }
  }
  for (int32_t ax1 = 0; ax1 < 25; ++ax1) {
    for (int32_t ax2 = 0; ax2 < 5; ++ax2) {
      for (int32_t ax3 = 0; ax3 < 64; ++ax3) {
        int32_t cse_var_5 = (((ax1 * 320) + (ax2 * 64)) + ax3);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_5] = (((int32_t*)DepthwiseConv2d_let)[cse_var_5] - ((int32_t*)fused_nn_conv2d_constant_7_let)[ax3]);
      }
    }
  }
  for (int32_t ax1_1 = 0; ax1_1 < 25; ++ax1_1) {
    for (int32_t ax2_1 = 0; ax2_1 < 5; ++ax2_1) {
      for (int32_t ax3_1 = 0; ax3_1 < 64; ++ax3_1) {
        int32_t cse_var_6 = (((ax1_1 * 320) + (ax2_1 * 64)) + ax3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_6] = (((int32_t*)DepthwiseConv2d_let)[cse_var_6] + ((int32_t*)fused_nn_conv2d_subtract_constant_7_let)[ax3_1]);
      }
    }
  }
  for (int32_t i1_1 = 0; i1_1 < 25; ++i1_1) {
    for (int32_t i2_1 = 0; i2_1 < 5; ++i2_1) {
      for (int32_t i3_1 = 0; i3_1 < 64; ++i3_1) {
        int32_t cse_var_7 = (((i1_1 * 320) + (i2_1 * 64)) + i3_1);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_7] = ((int32_t)(((((int64_t)((int32_t*)DepthwiseConv2d_let)[cse_var_7]) * ((int64_t)((int32_t*)fused_nn_conv2d_subtract_add_constant_21_let)[i3_1])) + ((int64_t)1 << ((int64_t)((((int32_t*)fused_nn_conv2d_subtract_add_constant_23_let)[i3_1] + 31) - 1)))) >> ((int64_t)(((int32_t*)fused_nn_conv2d_subtract_add_constant_23_let)[i3_1] + 31))));
      }
    }
  }
  for (int32_t ax1_2 = 0; ax1_2 < 25; ++ax1_2) {
    for (int32_t ax2_2 = 0; ax2_2 < 5; ++ax2_2) {
      for (int32_t ax3_2 = 0; ax3_2 < 64; ++ax3_2) {
        int32_t cse_var_8 = (((ax1_2 * 320) + (ax2_2 * 64)) + ax3_2);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_8] = (((int32_t*)DepthwiseConv2d_let)[cse_var_8] - 128);
      }
    }
  }
  for (int32_t i1_2 = 0; i1_2 < 25; ++i1_2) {
    for (int32_t i2_2 = 0; i2_2 < 5; ++i2_2) {
      for (int32_t i3_2 = 0; i3_2 < 64; ++i3_2) {
        int32_t cse_var_9 = (((i1_2 * 320) + (i2_2 * 64)) + i3_2);
        int32_t v_ = ((int32_t*)DepthwiseConv2d_let)[cse_var_9];
        int32_t v__1 = (v_) < (127) ? (v_) : (127);
        ((int32_t*)DepthwiseConv2d_let)[cse_var_9] = ((v__1) > (-128) ? (v__1) : (-128));
      }
    }
  }
  for (int32_t ax1_3 = 0; ax1_3 < 25; ++ax1_3) {
    for (int32_t ax2_3 = 0; ax2_3 < 5; ++ax2_3) {
      for (int32_t ax3_3 = 0; ax3_3 < 64; ++ax3_3) {
        int32_t cse_var_10 = (((ax1_3 * 320) + (ax2_3 * 64)) + ax3_3);
        T_cast[cse_var_10] = ((int8_t)((int32_t*)DepthwiseConv2d_let)[cse_var_10]);
      }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_dense_subtract_add_fixed_point_multiply_add_clip_subtract_cast_multiply(int8_t* p0, float* T_multiply, uint8_t* global_const_workspace_26_var, uint8_t* global_workspace_27_var) {
  void* fused_nn_dense_subtract_constant_let = (&(global_const_workspace_26_var[33600]));
  void* fused_nn_dense_constant_let = (&(global_const_workspace_26_var[33664]));
  void* fused_constant_9_let = (&(global_const_workspace_26_var[18944]));
  void* T_matmul_NT_let = (&(global_workspace_27_var[32320]));
  void* T_cast_let = (&(global_workspace_27_var[32384]));
  for (int32_t j = 0; j < 13; ++j) {
    ((int32_t*)T_matmul_NT_let)[j] = 0;
    for (int32_t k = 0; k < 64; ++k) {
      ((int32_t*)T_matmul_NT_let)[j] = (((int32_t*)T_matmul_NT_let)[j] + (((int32_t)p0[k]) * ((int32_t)((int8_t*)fused_constant_9_let)[((j * 64) + k)])));
    }
  }
  for (int32_t ax1 = 0; ax1 < 13; ++ax1) {
    ((int32_t*)T_matmul_NT_let)[ax1] = (((int32_t*)T_matmul_NT_let)[ax1] - ((int32_t*)fused_nn_dense_constant_let)[ax1]);
  }
  for (int32_t ax1_1 = 0; ax1_1 < 13; ++ax1_1) {
    ((int32_t*)T_matmul_NT_let)[ax1_1] = (((int32_t*)T_matmul_NT_let)[ax1_1] + ((int32_t*)fused_nn_dense_subtract_constant_let)[ax1_1]);
  }
  for (int32_t i1 = 0; i1 < 13; ++i1) {
    ((int32_t*)T_matmul_NT_let)[i1] = ((int32_t)(((((0 != 0) ? (((int64_t)((int32_t*)T_matmul_NT_let)[i1]) << ((int64_t)0)) : ((int64_t)((int32_t*)T_matmul_NT_let)[i1])) * (int64_t)2084188035) + ((int64_t)1 << ((int64_t)((8 + 31) - 1)))) >> ((int64_t)(8 + 31))));
  }
  for (int32_t ax1_2 = 0; ax1_2 < 13; ++ax1_2) {
    ((int32_t*)T_matmul_NT_let)[ax1_2] = (((int32_t*)T_matmul_NT_let)[ax1_2] + 48);
  }
  for (int32_t i1_1 = 0; i1_1 < 13; ++i1_1) {
    int32_t v_ = ((int32_t*)T_matmul_NT_let)[i1_1];
    int32_t v__1 = (v_) < (127) ? (v_) : (127);
    ((int32_t*)T_matmul_NT_let)[i1_1] = ((v__1) > (-128) ? (v__1) : (-128));
  }
  for (int32_t ax1_3 = 0; ax1_3 < 13; ++ax1_3) {
    ((int32_t*)T_matmul_NT_let)[ax1_3] = (((int32_t*)T_matmul_NT_let)[ax1_3] - 48);
  }
  for (int32_t ax1_4 = 0; ax1_4 < 13; ++ax1_4) {
    ((float*)T_cast_let)[ax1_4] = ((float)((int32_t*)T_matmul_NT_let)[ax1_4]);
  }
  for (int32_t ax1_5 = 0; ax1_5 < 13; ++ax1_5) {
    T_multiply[ax1_5] = (((float*)T_cast_let)[ax1_5] * 1.785739e-01f);
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_pad(int8_t* p0, int8_t* T_pad, uint8_t* global_const_workspace_6_var, uint8_t* global_workspace_7_var) {
  for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 27; ++ax0_ax1_fused) {
    for (int32_t ax2 = 0; ax2 < 7; ++ax2) {
      for (int32_t ax3_outer = 0; ax3_outer < 4; ++ax3_outer) {
        for (int32_t ax3_inner = 0; ax3_inner < 16; ++ax3_inner) {
          int32_t cse_var_2 = (ax2 * 64);
          int32_t cse_var_1 = (ax3_outer * 16);
          T_pad[((((ax0_ax1_fused * 448) + cse_var_2) + cse_var_1) + ax3_inner)] = (((((1 <= ax0_ax1_fused) && (ax0_ax1_fused < 26)) && (1 <= ax2)) && (ax2 < 6)) ? p0[(((((ax0_ax1_fused * 320) + cse_var_2) + cse_var_1) + ax3_inner) - 384)] : (int8_t)-128);
        }
      }
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_nn_softmax_divide_round_add_clip_cast(float* p0, int8_t* T_cast, uint8_t* global_const_workspace_28_var, uint8_t* global_workspace_29_var) {
  void* T_softmax_maxelem_let = (&(global_workspace_29_var[128]));
  void* T_softmax_exp_let = (&(global_workspace_29_var[64]));
  void* T_softmax_expsum_let = (&(global_workspace_29_var[128]));
  ((float*)T_softmax_maxelem_let)[0] = -3.402823e+38f;
  for (int32_t k = 0; k < 13; ++k) {
    float v_ = ((float*)T_softmax_maxelem_let)[0];
    float v__1 = p0[k];
    ((float*)T_softmax_maxelem_let)[0] = ((v_) > (v__1) ? (v_) : (v__1));
  }
  for (int32_t i1 = 0; i1 < 13; ++i1) {
    ((float*)T_softmax_exp_let)[i1] = expf((p0[i1] - ((float*)T_softmax_maxelem_let)[0]));
  }
  ((float*)T_softmax_expsum_let)[0] = 0.000000e+00f;
  for (int32_t k_1 = 0; k_1 < 13; ++k_1) {
    ((float*)T_softmax_expsum_let)[0] = (((float*)T_softmax_expsum_let)[0] + ((float*)T_softmax_exp_let)[k_1]);
  }
  for (int32_t i1_1 = 0; i1_1 < 13; ++i1_1) {
    ((float*)T_softmax_exp_let)[i1_1] = (((float*)T_softmax_exp_let)[i1_1] / ((float*)T_softmax_expsum_let)[0]);
  }
  for (int32_t ax1 = 0; ax1 < 13; ++ax1) {
    float v__2 = roundf((((float*)T_softmax_exp_let)[ax1] * 2.560000e+02f)) + -1.280000e+02f;
    float v__3 = (v__2) < (1.270000e+02f) ? (v__2) : (1.270000e+02f);
    T_cast[ax1] = ((int8_t)((v__3) > (-1.280000e+02f) ? (v__3) : (-1.280000e+02f)));
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws_fused_reshape_nn_pad(int8_t* p0, int8_t* T_pad, uint8_t* global_const_workspace_2_var, uint8_t* global_workspace_3_var) {
  for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 58; ++ax0_ax1_fused) {
    for (int32_t ax2 = 0; ax2 < 12; ++ax2) {
      T_pad[((ax0_ax1_fused * 12) + ax2)] = (((((4 <= ax0_ax1_fused) && (ax0_ax1_fused < 53)) && (1 <= ax2)) && (ax2 < 11)) ? p0[((((ax0_ax1_fused * 10) + ax2) + 449) % 490)] : (int8_t)100);
    }
  }
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif
TVM_DLL int32_t tvmgen_kws___tvm_main__(int8_t* input_buffer_var, int8_t* Identity_buffer_var, uint8_t* global_const_workspace_0_var, uint8_t* global_workspace_1_var) {
  void* sid_12_let = (&(global_workspace_1_var[0]));
  void* sid_11_let = (&(global_workspace_1_var[12096]));
  void* sid_14_let = (&(global_workspace_1_var[0]));
  void* sid_10_let = (&(global_workspace_1_var[32000]));
  void* sid_8_let = (&(global_workspace_1_var[12096]));
  void* sid_9_let = (&(global_workspace_1_var[0]));
  void* sid_7_let = (&(global_workspace_1_var[32000]));
  void* sid_2_let = (&(global_workspace_1_var[12096]));
  void* sid_3_let = (&(global_workspace_1_var[0]));
  void* sid_4_let = (&(global_workspace_1_var[32000]));
  void* sid_1_let = (&(global_workspace_1_var[20800]));
  void* sid_5_let = (&(global_workspace_1_var[12096]));
  void* sid_13_let = (&(global_workspace_1_var[40000]));
  void* sid_6_let = (&(global_workspace_1_var[0]));
  void* sid_15_let = (&(global_workspace_1_var[32256]));
  void* sid_16_let = (&(global_workspace_1_var[0]));
  if (tvmgen_kws_fused_reshape_nn_pad(input_buffer_var, sid_1_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast(sid_1_let, sid_2_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_pad(sid_2_let, sid_3_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_1(sid_3_let, sid_4_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_2(sid_4_let, sid_5_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_pad(sid_5_let, sid_6_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_3(sid_6_let, sid_7_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_4(sid_7_let, sid_8_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_pad(sid_8_let, sid_9_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_5(sid_9_let, sid_10_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_6(sid_10_let, sid_11_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_pad(sid_11_let, sid_12_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip_cast_7(sid_12_let, sid_13_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_conv2d_subtract_add_fixed_point_multiply_per_axis_add_clip(sid_13_let, sid_14_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_avg_pool2d_cast(sid_14_let, sid_15_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_dense_subtract_add_fixed_point_multiply_add_clip_subtract_cast_multiply(sid_15_let, sid_16_let, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  if (tvmgen_kws_fused_nn_softmax_divide_round_add_clip_cast(sid_16_let, Identity_buffer_var, global_const_workspace_0_var, global_workspace_1_var) != 0 ) return -1;
  return 0;
}

