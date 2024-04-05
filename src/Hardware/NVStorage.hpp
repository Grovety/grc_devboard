#pragma once
#include "IStorage.hpp"
#include "nvs.h"

/*!
 * \brief Interface to non-volatile storage.
 */
class NVStorage : public IStorage {
public:
  /*! \brief Initialization routine. */
  NVStorage();
  /*!
   * \brief Read train metadata from storage.
   * \param storage_namespace Storage namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   */
  void read(const char *storage_namespace, unsigned &qty,
            std::vector<float> &data) const override final;
  /*!
   * \brief Write train metadata to storage.
   * \param storage_namespace Storage namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   */
  void write(const char *storage_namespace, unsigned qty,
             const std::vector<float> &data) const override final;
  /*!
   * \brief Clear storage.
   * \param storage_namespace Storage namespace.
   */
  void clear(const char *storage_namespace) const override final;

private:
  /*!
   * \brief Read train metadata from NVS.
   * \param storage_namespace NVS namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   * \return Error value.
   */
  esp_err_t readNVS(const char *storage_namespace, unsigned &qty,
                    std::vector<float> &data) const;
  /*!
   * \brief Write train metadata to NVS.
   * \param storage_namespace NVS namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   * \return Error value.
   */
  esp_err_t writeNVS(const char *storage_namespace, unsigned qty,
                     const std::vector<float> &data) const;
};
