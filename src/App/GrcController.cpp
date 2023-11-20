#include "GrcController.hpp"
#include "IStorage.hpp"
#include "Status.hpp"
#include "common.hpp"
#include "custom_types.hpp"

constexpr char TAG[] = "GrcController";

static const char *get_grc_event_id_name(eGrcEventId id) {
  switch (id) {
  case eGrcEventId::TRAIN:
    return "TRAIN";
  case eGrcEventId::INFER:
    return "INFER";
  case eGrcEventId::LOAD_WGTS:
    return "LOAD_WGTS";
  case eGrcEventId::SAVE_WGTS:
    return "SAVE_WGTS";
  case eGrcEventId::CLEAR_WGTS:
    return "CLEAR_WGTS";
  case eGrcEventId::LOAD_SIGNAL:
    return "LOAD_SIGNAL";
  case eGrcEventId::NO_EVENT:
  default:
    return "NO_EVENT";
  }
}

GrcControllerQueues_t GrcControllerQueues;

static void grc_controller_task(void *pvParameters) {
  auto *p_grc = static_cast<BaseGrc *>(pvParameters);
  MatrixDyn signal;

  for (;;) {
    GrcEvent_t ev;
    if (xQueuePeek(GrcControllerQueues.xRxQueue, &ev, portMAX_DELAY) ==
        pdPASS) {
      LOGD(TAG, "recieve event: %s", get_grc_event_id_name(ev.id));
      switch (ev.id) {
      case eGrcEventId::TRAIN: {
        int train_category = -1;
        if (ev.data != nullptr) {
          int *p_cat = static_cast<int *>(ev.data);
          train_category = *p_cat;
          delete p_cat;
        }
        xEventGroupWaitBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        int category = -1;
        for (unsigned retry_count = 0; ; ++retry_count) {
          if (retry_count >= 5) {
            LOGE(TAG, "Failed to train category");
            abort();
          }
          category = p_grc->train(signal, train_category);
          if (category < 0) {
            LOGD(TAG, "Retry grc train, %d", retry_count);
            delayMS(2000);
          } else {
            break;
          }
        }
        signal = MatrixDyn();
        xEventGroupSetBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK);
      } break;
      case eGrcEventId::INFER: {
        xEventGroupWaitBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        int category = p_grc->inference(signal);
        signal = MatrixDyn();
        xEventGroupSetBits(xStatusEventGroup, STATUS_PRIORITY_SEMA_MSK);
        GrcReply_t reply = {eGrcReplyId::CATEGORY, category};
        xQueueSend(GrcControllerQueues.xTxQueue, &reply, portMAX_DELAY);
      } break;
      case eGrcEventId::LOAD_WGTS: {
        auto *stor = static_cast<IStorage *>(ev.data);
        std::vector<RT> buffer;
        unsigned cats = 0;
        stor->read(p_grc->getName(), cats, buffer);
        GrcReply_t reply = {eGrcReplyId::CATS_QTY, int(cats)};
        xQueueSend(GrcControllerQueues.xTxQueue, &reply, portMAX_DELAY);
        if (!p_grc->load(cats, buffer)) {
          LOGE(TAG, "Error loading weights to GRC");
        }
      } break;
      case eGrcEventId::SAVE_WGTS: {
        auto *stor = static_cast<IStorage *>(ev.data);
        std::vector<RT> buffer;
        unsigned cats = p_grc->save(buffer);
        GrcReply_t reply = {eGrcReplyId::CATS_QTY, int(cats)};
        xQueueSend(GrcControllerQueues.xTxQueue, &reply, portMAX_DELAY);
        stor->write(p_grc->getName(), cats, buffer);
      } break;
      case eGrcEventId::CLEAR_WGTS: {
        auto *stor = static_cast<IStorage *>(ev.data);
        stor->write(p_grc->getName(), 0, {});
        p_grc->clear();
        GrcReply_t reply = {eGrcReplyId::CATS_QTY, int(0)};
        xQueueSend(GrcControllerQueues.xTxQueue, &reply, portMAX_DELAY);
      } break;
      case eGrcEventId::LOAD_SIGNAL: {
        auto *data = static_cast<SignalData_t *>(ev.data);
        LOGI(TAG, "recieve data(%d, %d), sync_time_ms=%d", data->num_rows,
             data->num_cols, data->sync_time_ms);
        makeMatrix(signal, data->num_rows, data->num_cols, data->buffer);
        delete[] data->buffer;
        delete data;
      } break;
      default:
        break;
      }
      xQueueReceive(GrcControllerQueues.xRxQueue, &ev, portMAX_DELAY);
    }
  }
}

void initGrcController(BaseGrc *p_grc) {
  GrcControllerQueues.xTxQueue = xQueueCreate(10, sizeof(GrcReply_t));
  if (GrcControllerQueues.xTxQueue == NULL) {
    LOGE(TAG, "Error creating Tx queue");
  }
  GrcControllerQueues.xRxQueue = xQueueCreate(2, sizeof(GrcEvent_t));
  if (GrcControllerQueues.xRxQueue == NULL) {
    LOGE(TAG, "Error creating Rx queue");
  }

  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(grc_controller_task, "grc_controller_task",
                  configMINIMAL_STACK_SIZE + 1024 * 4, p_grc, 1, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating button event task");
    vTaskDelete(xHandle);
  }
}
