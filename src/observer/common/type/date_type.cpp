#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/value.h"

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATE && right.attr_type() == AttrType::DATE, "invalid type");
  return common::compare_int((void *)&left.value_.int_value_, (void *)&right.value_.int_value_);
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  val.set_date(data.c_str());
  return RC::SUCCESS;
}

RC DateType::to_string(const Value &val, string &result) const
{
  time_t     t  = static_cast<time_t>(val.value_.int_value_);
  struct tm *tm = localtime(&t);
  if (tm == nullptr) {
    LOG_WARN("failed to convert date to string");
    return RC::UNSUPPORTED;
  }
  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
  result = buffer;
  return RC::SUCCESS;
}

int DateType::cast_cost(AttrType type)
{
  if (type == AttrType::DATE) {
    return 0;
  }
  if (type == AttrType::CHARS) {
    return 1;
  }
  return INT32_MAX;
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  if (type == AttrType::DATE) {
    result = val;
    return RC::SUCCESS;
  } else if (type == AttrType::CHARS) {
    result.set_int(val.value_.int_value_);
    return RC::SUCCESS;
  }
  return RC::UNSUPPORTED;
}
