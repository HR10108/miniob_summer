#pragma once

#include "sql/operator/logical_operator.h"

/**
 * @brief 逻辑算子，用于执行delete语句
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(Table *table, Value *values, int value_amount, char *attribute);
  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }
  Table              *table() const { return table_; }
  Value              *values() const { return values_; }
  int                 value_amount() const { return value_amount_; }
  char               *attribute() const { return attribute_; }

private:
  Table *table_        = nullptr;
  Value *values_       = nullptr;
  int    value_amount_ = 0;
  char  *attribute_    = nullptr;

};