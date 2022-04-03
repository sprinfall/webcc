#include "gtest/gtest.h"

#include "webcc/url.h"

TEST(UrlTest, Basic) {
  webcc::Url url("http://example.com/path");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/path");
  EXPECT_EQ(url.query(), "");
}

TEST(UrlTest, Basic_TmpStr) {
  webcc::Url url{ std::string{ "http://example.com/path" } };

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/path");
  EXPECT_EQ(url.query(), "");
}

TEST(UrlTest, NoPath) {
  webcc::Url url("http://example.com");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "");
  EXPECT_EQ(url.query(), "");
}

TEST(UrlTest, NoPath2) {
  webcc::Url url("http://example.com/");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/");
  EXPECT_EQ(url.query(), "");
}

TEST(UrlTest, NoPath3) {
  webcc::Url url("http://example.com?key=value");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "");
  EXPECT_EQ(url.query(), "key=value");
}

TEST(UrlTest, NoPath4) {
  webcc::Url url("http://example.com/?key=value");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "example.com");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/");
  EXPECT_EQ(url.query(), "key=value");
}

TEST(UrlTest, NoScheme) {
  webcc::Url url("/path/to");

  EXPECT_EQ(url.scheme(), "");
  EXPECT_EQ(url.host(), "");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/path/to");
  EXPECT_EQ(url.query(), "");
}

TEST(UrlTest, NoScheme2) {
  webcc::Url url("/path/to?key=value");

  EXPECT_EQ(url.scheme(), "");
  EXPECT_EQ(url.host(), "");
  EXPECT_EQ(url.port(), "");
  EXPECT_EQ(url.path(), "/path/to");
  EXPECT_EQ(url.query(), "key=value");
}

TEST(UrlTest, Full) {
  webcc::Url url("https://localhost:3000/path/to?key=value");

  EXPECT_EQ(url.scheme(), "https");
  EXPECT_EQ(url.host(), "localhost");
  EXPECT_EQ(url.port(), "3000");
  EXPECT_EQ(url.path(), "/path/to");
  EXPECT_EQ(url.query(), "key=value");
}

TEST(UrlTest, IPv6) {
  webcc::Url url("http://[::1]:8080");

  EXPECT_EQ(url.scheme(), "http");
  EXPECT_EQ(url.host(), "[::1]");
  EXPECT_EQ(url.port(), "8080");
  EXPECT_EQ(url.path(), "");
  EXPECT_EQ(url.query(), "");
}

// TODO: Add cases for UrlQuery