#include "sql/operator/order_by_logical_operator.h"

OrderByLogicalOperator::OrderByLogicalOperator(std::vector<std::unique_ptr<Expression>> &&order_expressions)
{
  // 将排序表达式移动到基类的 expressions_ 中
  for (auto &expr : order_expressions) {
    expressions_.emplace_back(std::move(expr));
  }
}
