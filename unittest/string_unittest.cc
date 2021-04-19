#include "gtest/gtest.h"

#include <vector>

#include "boost/algorithm/string.hpp"

#include "webcc/string.h"

TEST(StringTest, Split) {
  std::vector<boost::string_view> parts;
  webcc::Split("GET /path/to HTTP/1.1", ' ', false, &parts);

  EXPECT_EQ(3, parts.size());
  EXPECT_EQ("GET", parts[0]);
  EXPECT_EQ("/path/to", parts[1]);
  EXPECT_EQ("HTTP/1.1", parts[2]);
}

TEST(StringTest, Split_TokenCompressOff) {
  std::string str = "one,two,,three,,";
  std::vector<boost::string_view> parts;

  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_off);

  webcc::Split(str, ',', false, &parts);

  EXPECT_EQ(6, parts.size());
  EXPECT_EQ("one", parts[0]);
  EXPECT_EQ("two", parts[1]);
  EXPECT_EQ("", parts[2]);
  EXPECT_EQ("three", parts[3]);
  EXPECT_EQ("", parts[4]);
  EXPECT_EQ("", parts[5]);
}

TEST(StringTest, Split_TokenCompressOn) {
  std::string str = "one,two,,three,,";
  std::vector<boost::string_view> parts;

  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_on);
  webcc::Split(str, ',', true, &parts);

  EXPECT_EQ(4, parts.size());
  EXPECT_EQ("one", parts[0]);
  EXPECT_EQ("two", parts[1]);
  EXPECT_EQ("three", parts[2]);
  EXPECT_EQ("", parts[3]);
}

TEST(StringTest, Split_NewLine) {
  std::vector<boost::string_view> lines;
  webcc::Split("line one\nline two\nline 3", '\n', false, &lines);

  EXPECT_EQ(3, lines.size());
  EXPECT_EQ("line one", lines[0]);
  EXPECT_EQ("line two", lines[1]);
  EXPECT_EQ("line 3", lines[2]);
}

TEST(StringTest, SplitKV) {
  const std::string str = "key=value";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_EQ(true, ok);
  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_OtherDelim) {
  const std::string str = "key:value";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, ':', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_Spaces) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_SpacesNoTrim) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', false, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ(" key ", key);
  EXPECT_EQ("  value ", value);
}

TEST(StringTest, SplitKV_NoKey) {
  const std::string str = "=value";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_NoValue) {
  const std::string str = "key=";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("", value);
}

TEST(StringTest, SplitKV_NoKeyNoValue) {
  const std::string str = "=";

  std::string key;
  std::string value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("", value);
}
