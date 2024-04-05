#include "kws.h"

#include "esp_log.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

static tflite::MicroMutableOpResolver<7> s_micro_op_resolver;
static tflite::MicroInterpreter *s_interpreter = nullptr;
static const char **s_labels = nullptr;
static size_t s_labels_num = 0;

constexpr int kTensorArenaSize = 23 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

bool kws_model_init_op_resolver() {
  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  if (s_micro_op_resolver.AddAveragePool2D() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddConv2D() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddDepthwiseConv2D() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddFullyConnected() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddRelu() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddSoftmax() != kTfLiteOk) {
    return false;
  }
  if (s_micro_op_resolver.AddReshape() != kTfLiteOk) {
    return false;
  }
  return true;
}

bool kws_model_init(const unsigned char* model_ptr, const char **labels, size_t labels_num) {
  if (s_interpreter) {
    ESP_LOGW(__FUNCTION__, "interpreter is already created");
    return true;
  }
  s_labels = labels;
  s_labels_num = labels_num;
  const tflite::Model *model = tflite::GetModel(model_ptr);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.",
                model->version(), TFLITE_SCHEMA_VERSION);
    return false;
  }

  // Build an interpreter to run the model with.
  s_interpreter = new tflite::MicroInterpreter(model, s_micro_op_resolver, tensor_arena, kTensorArenaSize);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = s_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return false;
  }

  // Get information about the memory area to use for the model's input.
  TfLiteTensor *model_input = s_interpreter->input(0);
  if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != (KWS_FEATURES_LEN)) ||
      (model_input->type != kTfLiteInt8)) {
    MicroPrintf("Bad input tensor parameters in model");
    return false;
  }

  return true;
}

void kws_model_release() {
  if (s_interpreter) {
    delete s_interpreter;
    s_interpreter = nullptr;
  }
  s_labels = nullptr;
  s_labels_num = 0;
}

static void quantize(const float *src, TfLiteTensor *tensor, size_t len) {
  for (int i = 0; i < len; i++)
    tensor->data.int8[i] =
        (int8_t)(src[i] / tensor->params.scale + tensor->params.zero_point);
}

static void dequantize(const TfLiteTensor *tensor, float *dst, size_t len) {
  for (int i = 0; i < len; i++) {
    dst[i] = (tensor->data.int8[i] - tensor->params.zero_point) *
             tensor->params.scale;
  }
}

static size_t argmax(float *array, size_t len) {
  size_t idx = 0;
  for (size_t i = 0; i < len; i++) {
    if (array[i] > array[idx])
      idx = i;
  }
  return idx;
}

void kws_get_category(int category, char *buffer, size_t len) {
  if (s_labels) {
    if (category < 0) {
      strncpy(buffer, s_labels[1], len);
    } else  if (category < s_labels_num) {
      strncpy(buffer, s_labels[category], len);
    }
  }
}

int kws_recognize_word(const float *src, size_t len) {
  if (!s_interpreter) {
    MicroPrintf("No interpreter");
    return -1;
  }
  // set input
  quantize(src, s_interpreter->input(0), len);

  TfLiteStatus invoke_status = s_interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf("Invoke failed");
    return -1;
  }

  // get inference
  float *out_buffer = new float[s_labels_num];
  dequantize(s_interpreter->output(0), out_buffer, s_labels_num);
  const size_t idx = argmax(out_buffer, s_labels_num);
  ESP_LOGD(__FUNCTION__, "category=%d(%f)", idx, out_buffer[idx]);
  int ret = -1;
  if (out_buffer[idx] > KWS_INFERENCE_THRESHOLD) {
    ret = idx;
  }
  delete[] out_buffer;
  return ret;
}
