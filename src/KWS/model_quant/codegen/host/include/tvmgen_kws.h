#ifndef TVMGEN_KWS_H_
#define TVMGEN_KWS_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Input tensor input size (in bytes) for TVM module "kws" 
 */
#define TVMGEN_KWS_INPUT_SIZE 490
/*!
 * \brief Output tensor Identity size (in bytes) for TVM module "kws" 
 */
#define TVMGEN_KWS_IDENTITY_SIZE 13
/*!
 * \brief Input tensor pointers for TVM module "kws" 
 */
struct tvmgen_kws_inputs {
  void* input;
};

/*!
 * \brief Output tensor pointers for TVM module "kws" 
 */
struct tvmgen_kws_outputs {
  void* Identity;
};

/*!
 * \brief entrypoint function for TVM module "kws"
 * \param inputs Input tensors for the module 
 * \param outputs Output tensors for the module 
 */
int32_t tvmgen_kws_run(
  struct tvmgen_kws_inputs* inputs,
  struct tvmgen_kws_outputs* outputs
);
/*!
 * \brief Workspace size for TVM module "kws" 
 */
#define TVMGEN_KWS_WORKSPACE_SIZE 48000

#ifdef __cplusplus
}
#endif

#endif // TVMGEN_KWS_H_
