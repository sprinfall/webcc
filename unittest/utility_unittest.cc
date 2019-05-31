#include "gtest/gtest.h"

#include "webcc/utility.h"

TEST(UtilityTest, SplitKV) {
  const std::string str = "Connection: Keep-Alive";

  std::string key;
  std::string value;

  bool ok = webcc::SplitKV(str, ':', &key, &value);

  EXPECT_EQ(true, ok);
  EXPECT_EQ("Connection", key);
  EXPECT_EQ("Keep-Alive", value);
}
