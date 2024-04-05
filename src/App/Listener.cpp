#include "Listener.hpp"
#include "Common.hpp"

static constexpr char TAG[] = "Listener";

ListenerQueues_t ListenerQueues;

EventGroupHandle_t xListenerEventGroup;

static void listener_task(void *pvParameters) {
  auto *p_sens = static_cast<ISensor *>(pvParameters);

  for (;;) {
    SignalData_t data;
    unsigned i = 0;
    if (xQueueReceive(ListenerQueues.xRxQueue, &data, portMAX_DELAY) ==
        pdPASS) {
      if (data.buffer != nullptr) {
        LOGD(TAG, "recieve data(%d, %d), sync_time_ms=%d", data.num_rows,
             data.num_cols, data.sync_time_ms);
        const TickType_t xTicksToWait = data.sync_time_ms / portTICK_PERIOD_MS;

        LOGD(TAG, "start listening");
        xEventGroupSetBits(xStatusEventGroup, STATUS_SENSOR_LISTENER_BUSY_MSK);
        auto xBits = xEventGroupGetBits(xListenerEventGroup);
        while (!(xBits & LISTENER_STOP_MSK)) {
          if (i >= data.num_cols) {
            xEventGroupClearBits(xListenerEventGroup, LISTENER_STOP_MSK);
            break;
          }
          p_sens->getData(&data.buffer[i * data.num_rows], data.num_rows);
          i++;
          xBits = xEventGroupWaitBits(xListenerEventGroup, LISTENER_STOP_MSK,
                                      pdTRUE, pdTRUE, xTicksToWait);
        }
        LOGD(TAG, "stop listening, i=%d", i);
        xEventGroupClearBits(xStatusEventGroup,
                             STATUS_SENSOR_LISTENER_BUSY_MSK);

        SignalData_t tx_data = data;
        tx_data.num_cols = i;
        xQueueSend(ListenerQueues.xTxQueue, &tx_data, portMAX_DELAY);
        LOGD(TAG, "send data(%d, %d), sync_time_ms=%d", tx_data.num_rows,
             tx_data.num_cols, tx_data.sync_time_ms);
      } else {
        LOGE(TAG, "Data buffer is not allocated");
      }
    }
  }
}

void initListener(ISensor *p_sens) {
  xListenerEventGroup = xEventGroupCreate();
  if (xListenerEventGroup == NULL) {
    LOGE(TAG, "Error creating xListenerEventGroup");
  }

  ListenerQueues.xTxQueue = xQueueCreate(1, sizeof(SignalData_t));
  if (ListenerQueues.xTxQueue == NULL) {
    LOGE(TAG, "Error creating Tx queue");
  }
  ListenerQueues.xRxQueue = xQueueCreate(1, sizeof(SignalData_t));
  if (ListenerQueues.xRxQueue == NULL) {
    LOGE(TAG, "Error creating Rx queue");
  }

  TaskHandle_t xHandle = NULL;
  auto xReturned =
      xTaskCreate(listener_task, "listener_task",
                  configMINIMAL_STACK_SIZE + 1024, p_sens, 2, &xHandle);
  if (xReturned != pdPASS) {
    LOGE(TAG, "Error creating task");
    vTaskDelete(xHandle);
  }
}
