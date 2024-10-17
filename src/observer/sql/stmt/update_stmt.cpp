/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

RC UpdateStmt::create(Db *db, UpdateSqlNode &update, Stmt *&stmt)
{
  const char *table_name = update.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || update.attribute_name.empty()) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, attribute_name=%p",
             db, table_name, update.attribute_name.c_str());
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  const TableMeta &table_meta = table->table_meta();

  // check the fields is valid
  const FieldMeta *field_meta = table_meta.field(update.attribute_name.c_str());
  if (nullptr == field_meta) {
    LOG_WARN("no such field. table_name=%s, field_name=%s", table_name, update.attribute_name.c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // check the field type
  const AttrType field_type = field_meta->type();
  const AttrType value_type = update.value.attr_type();

  if (field_type != value_type) {
    if (value_type == AttrType::CHARS && field_type == AttrType::DATE) {
      const char *date_str      = update.value.get_pointer();
      char       *date_str_copy = new char[strlen(date_str) + 1];
      strcpy(date_str_copy, date_str);
      if (!update.value.validate_date(date_str_copy)) {
        LOG_WARN("invalid date format. date_str=%s", date_str_copy);
        delete[] date_str_copy;
        return RC::INVALID_ARGUMENT;
      }
      // 设置转换后的日期
      update.value.set_date(date_str_copy);
      delete[] date_str_copy;
    } else {
      LOG_WARN("field type mismatch. field type=%d, value type=%d", field_type, value_type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

  // check the where condition
  std::unordered_map<std::string, Table *> table_map;
  table_map.insert({std::string(table_name), table});

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }
  stmt = new UpdateStmt(table, &update.value, 1, filter_stmt, const_cast<char *>(update.attribute_name.c_str()));
  return RC::SUCCESS;
}
