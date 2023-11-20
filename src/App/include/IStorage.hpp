#pragma once
#include "custom_types.hpp"
#include <vector>

/*!
 * \brief Interface to store data.
 */
class IStorage {
public:
  /*!
   * \brief Read train metadata from storage.
   * \param storage_namespace Storage namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   */
  virtual void read(const char *storage_namespace, unsigned &qty,
                    std::vector<RT> &data) const = 0;
  /*!
   * \brief Write train metadata to storage.
   * \param storage_namespace Storage namespace.
   * \param qty Number of trained categories.
   * \param data Train metadata.
   */
  virtual void write(const char *storage_namespace, unsigned qty,
                     const std::vector<RT> &data) const = 0;
  /*!
   * \brief Clear storage.
   * \param storage_namespace Storage namespace.
   */
  virtual void clear(const char *storage_namespace) const = 0;
};
