#include <algorithm>

#include "common/log/log.h"
#include "sql/operator/order_by_physical_operator.h"
#include "sql/expr/expression_tuple.h"
#include "sql/expr/composite_tuple.h"

using namespace std;
using namespace common;

OrderByPhysicalOperator::OrderByPhysicalOperator(std::vector<std::unique_ptr<Expression>> &&order_exprs)
    : order_exprs_(std::move(order_exprs))
{}

RC OrderByPhysicalOperator::open(Trx *trx)
{
  RC rc = RC::SUCCESS;

  // 检查是否只有一个子算子
  if (children_.size() != 1) {
    return RC::INTERNAL;
  }

  PhysicalOperator *child = children_[0].get();

  // 打开子算子
  rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // 获取所有数据
  while ((rc = child->next()) == RC::SUCCESS) {
    Tuple *tuple = child->current_tuple();
    if (nullptr == tuple) {
      rc = RC::INTERNAL;
      break;
    }

    // 将元组复制并保存
    std::unique_ptr<Tuple> tuple_copy(new ValueListTuple());
    rc = ValueListTuple::make(*tuple, static_cast<ValueListTuple &>(*tuple_copy));
    if (rc != RC::SUCCESS) {
      break;
    }
    sorted_tuples_.emplace_back(std::move(tuple_copy));
  }

  if (rc != RC::RECORD_EOF) {
    return rc;
  }

  // 对数据进行排序
  std::sort(sorted_tuples_.begin(),
      sorted_tuples_.end(),
      [this](const std::unique_ptr<Tuple> &a, const std::unique_ptr<Tuple> &b) {
        for (const auto &expr : order_exprs_) {
          Value val_a, val_b;
          expr->get_value(*a, val_a);
          expr->get_value(*b, val_b);

          int cmp_result = val_a.compare(val_b);
          if (cmp_result != 0) {
            // 判断排序顺序
            if (dynamic_cast<OrderExpr *>(expr.get())->is_ascending()) {
              return cmp_result < 0;
            } else {
              return cmp_result > 0;
            }
          }
        }
        return false;  // 所有排序字段都相等
      });

  current_index_ = 0;
  return RC::SUCCESS;
}

RC OrderByPhysicalOperator::next()
{
  if (current_index_ < sorted_tuples_.size()) {
    ++current_index_;
    return RC::SUCCESS;
  } else {
    return RC::RECORD_EOF;
  }
}

Tuple *OrderByPhysicalOperator::current_tuple()
{
  if (current_index_ == 0 || current_index_ > sorted_tuples_.size()) {
    return nullptr;
  }
  return sorted_tuples_[current_index_ - 1].get();
}

RC OrderByPhysicalOperator::close()
{
  sorted_tuples_.clear();
  if (children_.size() != 1) {
    return RC::INTERNAL;
  }
  return children_[0]->close();
}

std::string OrderByPhysicalOperator::param() const
{
  std::string params;
  for (const auto &expr : order_exprs_) {
    OrderExpr *order_expr = dynamic_cast<OrderExpr *>(expr.get());
    if (order_expr) {
      params += order_expr->child()->name();
      params += order_expr->is_ascending() ? " ASC" : " DESC";
      params += ", ";
    }
  }
  if (!params.empty()) {
    params.pop_back();  // 移除最后的逗号和空格
    params.pop_back();
  }
  return params;
}

RC OrderByPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  if (children_.empty()) {
    return RC::INTERNAL;
  }
  return children_[0]->tuple_schema(schema);
}
