
#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/expr/expression.h"

class OrderByLogicalOperator : public LogicalOperator
{
public:
  OrderByLogicalOperator(std::vector<std::unique_ptr<Expression>> &&expressions);

  virtual ~OrderByLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::ORDER_BY; }
};