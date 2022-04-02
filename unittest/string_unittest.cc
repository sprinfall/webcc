#include "gtest/gtest.h"

#include <algorithm>

#include "webcc/string.h"

TEST(StringTest, RandomAsciiString) {
  EXPECT_TRUE(webcc::RandomAsciiString(0).empty());

  std::string str = webcc::RandomAsciiString(10);
  EXPECT_EQ(str.size(), 10);
  EXPECT_TRUE(std::all_of(str.begin(), str.end(), std::isalnum));

  std::string str1 = webcc::RandomAsciiString(20);
  std::string str2 = webcc::RandomAsciiString(20);
  EXPECT_NE(str1, str2);
}

TEST(StringTest, Trim) {
  std::string_view sv = "   trim me  ";
  webcc::Trim(sv);
  EXPECT_EQ(sv, "trim me");
}

TEST(StringTest, Trim_Left) {
  std::string_view sv = "   trim me";
  webcc::Trim(sv);
  EXPECT_EQ(sv, "trim me");
}

TEST(StringTest, Trim_Right) {
  std::string_view sv = "trim me  ";
  webcc::Trim(sv);
  EXPECT_EQ(sv, "trim me");
}

TEST(StringTest, Trim_Empty) {
  std::string_view sv = "";
  webcc::Trim(sv);
  EXPECT_EQ(sv, "");
}

TEST(StringTest, Unquote) {
  EXPECT_EQ(webcc::Unquote(""), "");
  EXPECT_EQ(webcc::Unquote("\"\""), "");
  EXPECT_EQ(webcc::Unquote("\" test \""), " test ");
  EXPECT_EQ(webcc::Unquote(" \"test\" "), " \"test\" ");
}

TEST(StringTest, Split) {
  std::vector<std::string_view> parts;
  webcc::Split("GET /path/to HTTP/1.1", ' ', false, &parts);

  ASSERT_EQ(parts.size(), 3);
  EXPECT_EQ(parts[0], "GET");
  EXPECT_EQ(parts[1], "/path/to");
  EXPECT_EQ(parts[2], "HTTP/1.1");
}

TEST(StringTest, Split_TokenCompressOff) {
  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_off);
  std::vector<std::string_view> parts;
  webcc::Split("one,two,,three,,", ',', false, &parts);

  ASSERT_EQ(parts.size(), 6);
  EXPECT_EQ(parts[0], "one");
  EXPECT_EQ(parts[1], "two");
  EXPECT_EQ(parts[2], "");
  EXPECT_EQ(parts[3], "three");
  EXPECT_EQ(parts[4], "");
  EXPECT_EQ(parts[5], "");
}

TEST(StringTest, Split_TokenCompressOn) {
  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_on);
  std::vector<std::string_view> parts;
  webcc::Split("one,two,,three,,", ',', true, &parts);

  ASSERT_EQ(parts.size(), 4);
  EXPECT_EQ(parts[0], "one");
  EXPECT_EQ(parts[1], "two");
  EXPECT_EQ(parts[2], "three");
  EXPECT_EQ(parts[3], "");
}

TEST(StringTest, Split_TokensOnly) {
  // Token compress on
  std::vector<std::string_view> parts;
  webcc::Split(",,,,,", ',', true, &parts);
  ASSERT_EQ(parts.size(), 2);
  EXPECT_EQ(parts[0], "");
  EXPECT_EQ(parts[1], "");
  
  parts.clear();

  // Token compress off
  webcc::Split(",,,,,", ',', false, &parts);
  ASSERT_EQ(parts.size(), 6);
  EXPECT_EQ(parts[0], "");
  EXPECT_EQ(parts[1], "");
  EXPECT_EQ(parts[5], "");
}

TEST(StringTest, Split_NewLine) {
  std::vector<std::string_view> lines;
  webcc::Split("line one\nline two\nline 3", '\n', false, &lines);

  ASSERT_EQ(lines.size(), 3);
  EXPECT_EQ(lines[0], "line one");
  EXPECT_EQ(lines[1], "line two");
  EXPECT_EQ(lines[2], "line 3");
}

TEST(StringTest, SplitKV) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV("key=value", '=', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "key");
  EXPECT_EQ(value, "value");
}

TEST(StringTest, SplitKV_OtherDelim) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV("key:value", ':', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "key");
  EXPECT_EQ(value, "value");
}

TEST(StringTest, SplitKV_Spaces) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV(" key =  value ", '=', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "key");
  EXPECT_EQ(value, "value");
}

TEST(StringTest, SplitKV_SpacesNoTrim) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV(" key =  value ", '=', false, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, " key ");
  EXPECT_EQ(value, "  value ");
}

TEST(StringTest, SplitKV_NoKey) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV("=value", '=', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "");
  EXPECT_EQ(value, "value");
}

TEST(StringTest, SplitKV_NoValue) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV("key=", '=', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "key");
  EXPECT_EQ(value, "");
}

TEST(StringTest, SplitKV_NoKeyNoValue) {
  std::string_view key;
  std::string_view value;
  bool ok = webcc::SplitKV("=", '=', true, &key, &value);

  EXPECT_TRUE(ok);
  EXPECT_EQ(key, "");
  EXPECT_EQ(value, "");
}
