#include "sql/operator/update_physical_operator.h"
#include "common/log/log.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

RC UpdatePhysicalOperator::close() { return RC::SUCCESS; }

RC UpdatePhysicalOperator::next() { return RC::RECORD_EOF; }

// SCY 未测试
RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::SUCCESS;
  }

  // 打开子操作符，获取需要更新的记录
  std::unique_ptr<PhysicalOperator> &child = children_[0];
  RC                                 rc    = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("无法打开子操作符: %s", strrc(rc));
    return rc;
  }

  trx_ = trx;

  // 迭代子操作符，收集所有需要更新的记录
  while (OB_SUCC(rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      LOG_WARN("无法获取当前记录: %s", strrc(rc));
      return rc;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record   &record    = row_tuple->record();
    records_.emplace_back(record);
  }

  // 关闭子操作符
  child->close();

  // 获取表的元数据
  const TableMeta &table_meta = table_->table_meta();

  // 对收集到的记录进行删除并插入更新后的记录
  for (Record &record : records_) {
    // 删除旧记录
    rc = trx_->delete_record(table_, record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("删除记录失败: %s", strrc(rc));
      return rc;
    }

    // 获取原始数据
    char *data = record.data();

    // 准备新记录的值
    std::vector<Value> values;
    int                field_num = table_meta.field_num();

    for (int i = 0; i < field_num; ++i) {
      const FieldMeta *field_meta = table_meta.field(i);
      if (field_meta->visible()) {
        int      offset    = field_meta->offset();
        AttrType attr_type = field_meta->type();

        // 检查是否是要更新的字段
        if (0 == strcmp(field_meta->name(), attribute_)) {
          // 是要更新的字段，使用新值
          values.push_back(values_[0]);  // 假设只有一个新值
        } else {
          // 否则，从原始数据中获取值
          Value value;
          char *field_data = data + offset;
          switch (attr_type) {
            case AttrType::INTS: {
              int int_value = *(int *)field_data;
              value.set_int(int_value);
            } break;
            case AttrType::FLOATS: {
              float float_value = *(float *)field_data;
              value.set_float(float_value);
            } break;
            case AttrType::CHARS: {
              const char *char_value = field_data;
              value.set_string(char_value);
            } break;
            case AttrType::DATE: {
              int64_t date_value = *(int64_t *)field_data;
              value.set_long_int(date_value);
            } break;
            default: {
              LOG_WARN("未知的属性类型: %d", attr_type);
              return RC::UNSUPPORTED;
            }
          }
          values.push_back(value);
        }
      }
    }

    // 创建新的记录
    Record new_record;
    rc = table_->make_record(static_cast<int>(values.size()), values.data(), new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("创建新记录失败: %s", strrc(rc));
      return rc;
    }

    // 插入新记录
    rc = trx_->insert_record(table_, new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("插入新记录失败: %s", strrc(rc));
      delete[] new_record.data();  // 释放新记录的数据内存
      return rc;
    }

    // 插入成功后，释放新记录的数据内存
    delete[] new_record.data();
  }

  return RC::SUCCESS;
}
