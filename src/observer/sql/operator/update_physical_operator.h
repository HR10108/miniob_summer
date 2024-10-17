#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class UpdateStmt;

class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table, Value *values, int value_amount, char *attribute)
      : table_(table), values_(values), value_amount_(value_amount), attribute_(attribute)
  {}

  virtual ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  Table              *table_        = nullptr;
  Trx                *trx_          = nullptr;
  Value              *values_       = nullptr;
  int                 value_amount_ = 0;
  char               *attribute_    = nullptr;
  std::vector<Record> records_;
};
