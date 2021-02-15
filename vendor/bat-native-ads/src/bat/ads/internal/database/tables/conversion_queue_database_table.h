/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_CONVERSION_QUEUE_DATABASE_TABLE_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_CONVERSION_QUEUE_DATABASE_TABLE_H_

#include <string>

#include "bat/ads/ads_client.h"
#include "bat/ads/internal/conversions/conversion_queue_item_info.h"
#include "bat/ads/internal/database/database_table.h"
#include "bat/ads/mojom.h"
#include "bat/ads/result.h"

namespace ads {

using GetConversionQueueCallback =
    std::function<void(const Result, const ConversionQueueItemList&)>;

namespace database {
namespace table {

class ConversionQueue : public Table {
 public:
  ConversionQueue();

  ~ConversionQueue() override;

  void InsertOrUpdate(const ConversionQueueItemInfo& conversion_queue_item,
                      ResultCallback callback);

  void Delete(const ConversionQueueItemInfo& conversion_queue_item,
              ResultCallback callback);

  void GetAll(GetConversionQueueCallback callback);

  std::string get_table_name() const override;

  void Migrate(DBTransaction* transaction, const int to_version) override;

 private:
  void RunTransaction(const std::string& query,
                      GetConversionQueueCallback callback);

  void InsertOrUpdate(DBTransaction* transaction,
                      const ConversionQueueItemList& conversion_queue_items);

  int BindParameters(DBCommand* command,
                     const ConversionQueueItemList& conversion_queue_items);

  std::string BuildInsertOrUpdateQuery(
      DBCommand* command,
      const ConversionQueueItemList& conversion_queue_items);

  void OnGetConversionQueue(DBCommandResponsePtr response,
                            GetConversionQueueCallback callback);

  ConversionQueueItemInfo GetFromRecord(DBRecord* record) const;

  void CreateTableV10(DBTransaction* transaction);
  void MigrateToV10(DBTransaction* transaction);
};

}  // namespace table
}  // namespace database
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_DATABASE_TABLES_CONVERSION_QUEUE_DATABASE_TABLE_H_
