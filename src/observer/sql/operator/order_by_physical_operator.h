#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/expr/expression.h"
#include "sql/expr/tuple_cell.h"
#include <algorithm>
#pragma once

#include "sql/operator/physical_operator.h"
#include "sql/expr/expression.h"
#include <vector>
#include <memory>
#include <algorithm>

class OrderByPhysicalOperator : public PhysicalOperator
{
public:
  OrderByPhysicalOperator(std::vector<std::unique_ptr<Expression>> &&order_exprs);
  virtual ~OrderByPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::ORDER_BY; }

  RC     open(Trx *trx) override;
  RC     next() override;
  RC     close() override;
  Tuple *current_tuple() override;

  std::string name() const override { return "OrderBy"; }
  std::string param() const override;

  RC tuple_schema(TupleSchema &schema) const override;

private:
  std::vector<std::unique_ptr<Expression>> order_exprs_;        // 排序表达式列表
  std::vector<std::unique_ptr<Tuple>>      sorted_tuples_;      // 排序后的元组列表
  size_t                                   current_index_ = 0;  // 当前元组的索引
};
