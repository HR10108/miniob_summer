#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table *table, Value *values, int value_amount, char *attribute)
    : table_(table), values_(values), value_amount_(value_amount), attribute_(attribute)
{}