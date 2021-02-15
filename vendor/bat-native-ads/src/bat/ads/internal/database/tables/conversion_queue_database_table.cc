/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/database/tables/conversion_queue_database_table.h"

#include <functional>
#include <utility>

#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/database/database_statement_util.h"
#include "bat/ads/internal/database/database_table_util.h"
#include "bat/ads/internal/database/database_util.h"
#include "bat/ads/internal/logging.h"

namespace ads {
namespace database {
namespace table {

namespace {
const char kTableName[] = "conversion_queue";
}  // namespace

ConversionQueue::ConversionQueue() = default;

ConversionQueue::~ConversionQueue() = default;

void ConversionQueue::InsertOrUpdate(
    const ConversionQueueItemInfo& conversion_queue_item,
    ResultCallback callback) {
  DBTransactionPtr transaction = DBTransaction::New();

  InsertOrUpdate(transaction.get(), {conversion_queue_item});

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

void ConversionQueue::Delete(
    const ConversionQueueItemInfo& conversion_queue_item,
    ResultCallback callback) {
  DBTransactionPtr transaction = DBTransaction::New();

  const std::string query = base::StringPrintf(
      "DELETE FROM %s "
      "WHERE creative_instance_id = '%s'",
      get_table_name().c_str(),
      conversion_queue_item.creative_instance_id.c_str());

  DBCommandPtr command = DBCommand::New();
  command->type = DBCommand::Type::EXECUTE;
  command->command = query;

  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction),
      std::bind(&OnResultCallback, std::placeholders::_1, callback));
}

void ConversionQueue::GetAll(GetConversionQueueCallback callback) {
  const std::string query = base::StringPrintf(
      "SELECT "
      "cq.campaign_id, "
      "cq.creative_set_id, "
      "cq.creative_instance_id, "
      "cq.advertiser_id, "
      "cq.timestamp "
      "FROM %s AS cq "
      "ORDER BY timestamp ASC",
      get_table_name().c_str());

  RunTransaction(query, callback);
}

std::string ConversionQueue::get_table_name() const {
  return kTableName;
}

void ConversionQueue::Migrate(DBTransaction* transaction,
                              const int to_version) {
  DCHECK(transaction);

  switch (to_version) {
    case 10: {
      MigrateToV10(transaction);
      break;
    }

    default: {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void ConversionQueue::RunTransaction(const std::string& query,
                                     GetConversionQueueCallback callback) {
  DBCommandPtr command = DBCommand::New();
  command->type = DBCommand::Type::READ;
  command->command = query;

  command->record_bindings = {
      DBCommand::RecordBindingType::STRING_TYPE,  // campaign_id
      DBCommand::RecordBindingType::STRING_TYPE,  // creative_set_id
      DBCommand::RecordBindingType::STRING_TYPE,  // creative_instance_id
      DBCommand::RecordBindingType::STRING_TYPE,  // advertiser_id
      DBCommand::RecordBindingType::INT64_TYPE    // timestamp
  };

  DBTransactionPtr transaction = DBTransaction::New();
  transaction->commands.push_back(std::move(command));

  AdsClientHelper::Get()->RunDBTransaction(
      std::move(transaction), std::bind(&ConversionQueue::OnGetConversionQueue,
                                        this, std::placeholders::_1, callback));
}

void ConversionQueue::InsertOrUpdate(
    DBTransaction* transaction,
    const ConversionQueueItemList& conversion_queue_items) {
  DCHECK(transaction);

  if (conversion_queue_items.empty()) {
    return;
  }

  DBCommandPtr command = DBCommand::New();
  command->type = DBCommand::Type::RUN;
  command->command =
      BuildInsertOrUpdateQuery(command.get(), conversion_queue_items);

  transaction->commands.push_back(std::move(command));
}

int ConversionQueue::BindParameters(
    DBCommand* command,
    const ConversionQueueItemList& conversion_queue_items) {
  DCHECK(command);

  int count = 0;

  int index = 0;
  for (const auto& conversion_queue_item : conversion_queue_items) {
    BindString(command, index++, conversion_queue_item.campaign_id);
    BindString(command, index++, conversion_queue_item.creative_set_id);
    BindString(command, index++, conversion_queue_item.creative_instance_id);
    BindString(command, index++, conversion_queue_item.advertiser_id);
    BindInt64(
        command, index++,
        static_cast<int64_t>(conversion_queue_item.timestamp.ToDoubleT()));

    count++;
  }

  return count;
}

std::string ConversionQueue::BuildInsertOrUpdateQuery(
    DBCommand* command,
    const ConversionQueueItemList& conversion_queue_items) {
  DCHECK(command);

  const int count = BindParameters(command, conversion_queue_items);

  return base::StringPrintf(
      "INSERT OR REPLACE INTO %s "
      "(campaign_id, "
      "creative_set_id, "
      "creative_instance_id, "
      "advertiser_id, "
      "timestamp) VALUES %s",
      get_table_name().c_str(),
      BuildBindingParameterPlaceholders(5, count).c_str());
}

void ConversionQueue::OnGetConversionQueue(
    DBCommandResponsePtr response,
    GetConversionQueueCallback callback) {
  if (!response || response->status != DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Failed to get conversion queue");
    callback(Result::FAILED, {});
    return;
  }

  ConversionQueueItemList conversion_queue_items;

  for (const auto& record : response->result->get_records()) {
    ConversionQueueItemInfo info = GetFromRecord(record.get());
    conversion_queue_items.push_back(info);
  }

  callback(Result::SUCCESS, conversion_queue_items);
}

ConversionQueueItemInfo ConversionQueue::GetFromRecord(DBRecord* record) const {
  ConversionQueueItemInfo info;

  info.campaign_id = ColumnString(record, 0);
  info.creative_set_id = ColumnString(record, 1);
  info.creative_instance_id = ColumnString(record, 2);
  info.advertiser_id = ColumnString(record, 3);
  info.timestamp =
      base::Time::FromDoubleT(static_cast<double>(ColumnInt64(record, 4)));

  return info;
}

void ConversionQueue::CreateTableV10(DBTransaction* transaction) {
  DCHECK(transaction);

  const std::string query = base::StringPrintf(
      "CREATE TABLE %s "
      "(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "campaign_id TEXT NOT NULL, "
      "creative_set_id TEXT NOT NULL, "
      "creative_instance_id TEXT NOT NULL, "
      "advertiser_id TEXT NOT NULL, "
      "timestamp TIMESTAMP NOT NULL)",
      get_table_name().c_str());

  DBCommandPtr command = DBCommand::New();
  command->type = DBCommand::Type::EXECUTE;
  command->command = query;

  transaction->commands.push_back(std::move(command));
}

void ConversionQueue::MigrateToV10(DBTransaction* transaction) {
  DCHECK(transaction);

  util::Drop(transaction, get_table_name());

  CreateTableV10(transaction);
}

}  // namespace table
}  // namespace database
}  // namespace ads
