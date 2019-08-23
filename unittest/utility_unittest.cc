#include "gtest/gtest.h"

#include "webcc/utility.h"

TEST(UtilityTest, SplitKV) {
  const std::string str = "key=value";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value);

  EXPECT_EQ(true, ok);
  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, SplitKV_OtherDelim) {
  const std::string str = "key:value";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, ':', &key, &value);
  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, SplitKV_Spaces) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value);
  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, SplitKV_SpacesNoTrim) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value, false);
  EXPECT_TRUE(ok);

  EXPECT_EQ(" key ", key);
  EXPECT_EQ("  value ", value);
}

TEST(UtilityTest, SplitKV_NoKey) {
  const std::string str = "=value";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value);
  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, SplitKV_NoValue) {
  const std::string str = "key=";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value);
  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("", value);
}

TEST(UtilityTest, SplitKV_NoKeyNoValue) {
  const std::string str = "=";

  std::string key;
  std::string value;

  bool ok = webcc::utility::SplitKV(str, '=', &key, &value);
  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("", value);
}
