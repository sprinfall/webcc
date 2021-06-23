#include "gtest/gtest.h"

#include "webcc/string.h"

TEST(StringTest, Trim) {
  std::string str = "   trim me  ";
  webcc::string_view sv = str;
  webcc::Trim(sv);
  EXPECT_EQ("trim me", sv);
}

TEST(StringTest, Trim_Left) {
  std::string str = "   trim me";
  webcc::string_view sv = str;
  webcc::Trim(sv);
  EXPECT_EQ("trim me", sv);
}

TEST(StringTest, Trim_Right) {
  std::string str = "trim me  ";
  webcc::string_view sv = str;
  webcc::Trim(sv);
  EXPECT_EQ("trim me", sv);
}

TEST(StringTest, Trim_Empty) {
  std::string str = "";
  webcc::string_view sv = str;
  webcc::Trim(sv);
  EXPECT_EQ("", sv);
}

TEST(StringTest, Split) {
  std::vector<webcc::string_view> parts;
  webcc::Split("GET /path/to HTTP/1.1", ' ', false, &parts);

  EXPECT_EQ(3, parts.size());
  EXPECT_EQ("GET", parts[0]);
  EXPECT_EQ("/path/to", parts[1]);
  EXPECT_EQ("HTTP/1.1", parts[2]);
}

TEST(StringTest, Split_TokenCompressOff) {
  std::string str = "one,two,,three,,";
  std::vector<webcc::string_view> parts;

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
  std::vector<webcc::string_view> parts;

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

TEST(StringTest, Split_TokensOnly) {
  std::string str = ",,,,,";
  std::vector<webcc::string_view> parts;

  // Token compress on
  webcc::Split(str, ',', true, &parts);
  EXPECT_EQ(2, parts.size());
  EXPECT_EQ("", parts[0]);
  EXPECT_EQ("", parts[1]);
  
  parts.clear();

  // Token compress off
  webcc::Split(str, ',', false, &parts);
  EXPECT_EQ(6, parts.size());
  EXPECT_EQ("", parts[0]);
  EXPECT_EQ("", parts[1]);
  EXPECT_EQ("", parts[5]);
}

TEST(StringTest, Split_NewLine) {
  std::vector<webcc::string_view> lines;
  webcc::Split("line one\nline two\nline 3", '\n', false, &lines);

  EXPECT_EQ(3, lines.size());
  EXPECT_EQ("line one", lines[0]);
  EXPECT_EQ("line two", lines[1]);
  EXPECT_EQ("line 3", lines[2]);
}

TEST(StringTest, SplitKV) {
  const std::string str = "key=value";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_EQ(true, ok);
  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_OtherDelim) {
  const std::string str = "key:value";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, ':', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_Spaces) {
  const std::string str = " key =  value ";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_SpacesNoTrim) {
  const std::string str = " key =  value ";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', false, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ(" key ", key);
  EXPECT_EQ("  value ", value);
}

TEST(StringTest, SplitKV_NoKey) {
  const std::string str = "=value";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("value", value);
}

TEST(StringTest, SplitKV_NoValue) {
  const std::string str = "key=";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("", value);
}

TEST(StringTest, SplitKV_NoKeyNoValue) {
  const std::string str = "=";

  webcc::string_view key;
  webcc::string_view value;
  bool ok = webcc::SplitKV(str, '=', true, &key, &value);

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("", value);
}
