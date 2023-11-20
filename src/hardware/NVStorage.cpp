#include "NVStorage.hpp"
#include "esp_log.h"
#include "nvs_flash.h"

#define GRCS_QTY_NAME "GRCs_qty"
#define GRCS_DATA_NAME "GRCs_data"

constexpr char TAG[] = "NVStorage";

NVStorage::NVStorage() {
  ESP_LOGD(TAG, "open NVS");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void NVStorage::read(const char *storage_namespace, unsigned &qty,
                     std::vector<RT> &data) const {
  ESP_ERROR_CHECK(readNVS(storage_namespace, qty, data));
}

void NVStorage::write(const char *storage_namespace, unsigned qty,
                      const std::vector<RT> &data) const {
  ESP_ERROR_CHECK(writeNVS(storage_namespace, qty, data));
}

void NVStorage::clear(const char *storage_namespace) const {
  ESP_ERROR_CHECK(writeNVS(storage_namespace, 0, {}));
}

esp_err_t NVStorage::readNVS(const char *storage_namespace, unsigned &qty,
                             std::vector<RT> &data) const {
  nvs_handle_t my_handle;

  esp_err_t err = nvs_open(storage_namespace, NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
    return err;

  {
    int32_t grcs_qty = 0;
    err = nvs_get_i32(my_handle, GRCS_QTY_NAME, &grcs_qty);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      return err;
    if (grcs_qty < 0) {
      ESP_LOGW(TAG, "got invalid grcs_qty");
      qty = 0;
      ESP_ERROR_CHECK(writeNVS(storage_namespace, 0, {}));
      return ESP_OK;
    } else {
      qty = grcs_qty;
    }
  }

  size_t required_size = 0;
  err = nvs_get_blob(my_handle, GRCS_DATA_NAME, NULL, &required_size);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    return err;

  if (required_size != 0) {
    data.resize(required_size / sizeof(RT));
    err = nvs_get_blob(my_handle, GRCS_DATA_NAME, data.data(), &required_size);
    if (err != ESP_OK)
      return err;

    ESP_LOGD(TAG, "read %d values", data.size());
  } else {
    ESP_LOGW(TAG, "nothing saved yet");
  }

  nvs_close(my_handle);
  ESP_LOGI(TAG, "read NVS: %d", qty);
  return ESP_OK;
}

esp_err_t NVStorage::writeNVS(const char *storage_namespace, unsigned qty,
                              const std::vector<RT> &data) const {
  ESP_LOGI(TAG, "write NVS: %d", qty);
  nvs_handle_t my_handle;

  esp_err_t err = nvs_open(storage_namespace, NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
    return err;

  err = nvs_set_blob(my_handle, GRCS_DATA_NAME, data.data(),
                     data.size() * sizeof(RT));
  if (err != ESP_OK)
    return err;
  err = nvs_set_i32(my_handle, GRCS_QTY_NAME, qty);
  if (err != ESP_OK)
    return err;

  err = nvs_commit(my_handle);
  if (err != ESP_OK)
    return err;

  nvs_close(my_handle);
  return ESP_OK;
}
