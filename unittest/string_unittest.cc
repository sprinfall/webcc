#include "gtest/gtest.h"

#include <vector>

#include "webcc/string.h"

TEST(StringTest, iequals) {
  EXPECT_TRUE(webcc::iequals("", ""));
  EXPECT_TRUE(webcc::iequals("abc", "abc"));
  EXPECT_TRUE(webcc::iequals("ABC", "abc"));
  EXPECT_TRUE(webcc::iequals("123", "123"));

  EXPECT_FALSE(webcc::iequals("abc", "def"));
  EXPECT_FALSE(webcc::iequals("abc", "abcdef"));
}

TEST(StringTest, starts_with) {
  EXPECT_TRUE(webcc::starts_with("123", "1"));
  EXPECT_TRUE(webcc::starts_with("123", "12"));
  EXPECT_TRUE(webcc::starts_with("123", "123"));
  EXPECT_TRUE(webcc::starts_with(" 123", " "));

  EXPECT_FALSE(webcc::starts_with("123", ""));
  EXPECT_FALSE(webcc::starts_with("123", "2"));
}

TEST(StringTest, split) {
  std::vector<std::string> parts;
  webcc::split(parts, "GET /path/to HTTP/1.1");

  EXPECT_EQ(3, parts.size());
  EXPECT_EQ("GET", parts[0]);
  EXPECT_EQ("/path/to", parts[1]);
  EXPECT_EQ("HTTP/1.1", parts[2]);
}

TEST(StringTest, split_token_compress_off) {
  std::string str = "one,two,,three,,";
  std::vector<std::string> parts;

  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_off);

  webcc::split(parts, str, ',', false);

  EXPECT_EQ(6, parts.size());
  EXPECT_EQ("one", parts[0]);
  EXPECT_EQ("two", parts[1]);
  EXPECT_EQ("", parts[2]);
  EXPECT_EQ("three", parts[3]);
  EXPECT_EQ("", parts[4]);
  EXPECT_EQ("", parts[5]);
}

TEST(StringTest, split_token_compress_on) {
  std::string str = "one,two,,three,,";
  std::vector<std::string> parts;

  // Same as:
  //   boost::split(parts, str, boost::is_any_of(","),
  //                boost::token_compress_on);
  webcc::split(parts, str, ',', true);

  EXPECT_EQ(4, parts.size());
  EXPECT_EQ("one", parts[0]);
  EXPECT_EQ("two", parts[1]);
  EXPECT_EQ("three", parts[2]);
  EXPECT_EQ("", parts[3]);
}

TEST(StringTest, split_new_line) {
  std::vector<std::string> lines;
  webcc::split(lines, "line one\nline two\nline 3", '\n');

  EXPECT_EQ(3, lines.size());
  EXPECT_EQ("line one", lines[0]);
  EXPECT_EQ("line two", lines[1]);
  EXPECT_EQ("line 3", lines[2]);
}

TEST(UtilityTest, split_kv) {
  const std::string str = "key=value";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=');

  EXPECT_EQ(true, ok);
  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, split_kv_other_delim) {
  const std::string str = "key:value";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, ':');

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, split_kv_spaces) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=');

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, split_kv_spaces_no_trim) {
  const std::string str = " key =  value ";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=', false);

  EXPECT_TRUE(ok);

  EXPECT_EQ(" key ", key);
  EXPECT_EQ("  value ", value);
}

TEST(UtilityTest, split_kv_no_key) {
  const std::string str = "=value";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=');

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("value", value);
}

TEST(UtilityTest, split_kv_no_value) {
  const std::string str = "key=";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=');

  EXPECT_TRUE(ok);

  EXPECT_EQ("key", key);
  EXPECT_EQ("", value);
}

TEST(UtilityTest, split_kv_no_key_no_value) {
  const std::string str = "=";

  std::string key;
  std::string value;
  bool ok = webcc::split_kv(key, value, str, '=');

  EXPECT_TRUE(ok);

  EXPECT_EQ("", key);
  EXPECT_EQ("", value);
}
