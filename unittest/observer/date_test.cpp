
#include "common/type/date_type.h"
#include "common/type/char_type.h"
#include "common/value.h"
#include "common/rc.h"
#include "gtest/gtest.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

TEST(DateTypeTest, date_type_test)
{
  //   Value(const char *s, int len = 0);
  char     s[] = "2021-09-01";
  string   date_string;
  DateType date_type;
  CharType char_type;
  Value    val(s);
  Value    val1;
  EXPECT_EQ(val.attr_type(), AttrType::CHARS);
  char_type.to_string(val, date_string);
  EXPECT_EQ(date_string, "2021-09-01");
  string str;
  string str1;
  Value  result;
  RC     rc  = val.cast_to(val, AttrType::DATE, result);
  RC     rc1 = date_type.to_string(result, str);
  val1.set_date(s);
  RC rc2 = date_type.to_string(val1, str1);
  printf("str: %s\n", str.c_str());
  printf("str1: %s\n", str1.c_str());
  EXPECT_EQ(str1, "2021-09-01");
  EXPECT_EQ(rc, RC::SUCCESS);
  EXPECT_EQ(rc1, RC::SUCCESS);
  EXPECT_EQ(rc2, RC::SUCCESS);
  EXPECT_EQ(str, "2021-09-01");
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}