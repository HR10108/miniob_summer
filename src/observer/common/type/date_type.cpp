#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/value.h"

int compare_int64_t(const void *left, const void *right)
{
  int64_t l = *(int64_t *)left;
  int64_t r = *(int64_t *)right;
  if (l < r) {
    return -1;
  } else if (l > r) {
    return 1;
  }
  return 0;
}

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATE && right.attr_type() == AttrType::DATE, "invalid type");
  // 使用 compare_int 进行比较，并强制转换为 int64_t
  return compare_int64_t((void *)&left.value_.long_int_value_, (void *)&right.value_.long_int_value_);
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  val.set_date(data.c_str());
  return RC::SUCCESS;
}

RC DateType::to_string(const Value &val, string &result) const
{
  // 使用 int64_t 类型的时间戳
  int64_t   t = static_cast<int64_t>(val.value_.long_int_value_);
  struct tm tm;
  if (localtime_r((const time_t *)&t, &tm) == nullptr) {
    LOG_WARN("failed to convert date to string");
    return RC::UNSUPPORTED;
  }
  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
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
    string str;
    RC     rc = to_string(val, str);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    result.set_string(str.c_str());
    return RC::SUCCESS;
  }
  return RC::UNSUPPORTED;
}

// // 新增的方法，用于将 int64_t 类型的日期转换为字符串
// RC DateType::to_string(const Value &val, Value &result) const
// {
//   string str;
//   RC     rc = to_string(val, str);
//   if (rc != RC::SUCCESS) {
//     return rc;
//   }
//   result.set_string(str.c_str());
//   return RC::SUCCESS;
// }
