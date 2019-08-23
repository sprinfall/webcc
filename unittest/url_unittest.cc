#include "gtest/gtest.h"

#include "webcc/url.h"

TEST(UrlTest, Basic) {
  webcc::Url url("http://example.com/path");

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("/path", url.path());
  EXPECT_EQ("", url.query());
}

TEST(UrlTest, NoPath) {
  webcc::Url url("http://example.com");

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("", url.query());
}

TEST(UrlTest, NoPath2) {
  webcc::Url url("http://example.com/");

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("/", url.path());
  EXPECT_EQ("", url.query());
}

TEST(UrlTest, NoPath3) {
  webcc::Url url("http://example.com?key=value");

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(UrlTest, NoPath4) {
  webcc::Url url("http://example.com/?key=value");

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("/", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(UrlTest, NoScheme) {
  webcc::Url url("/path/to");

  EXPECT_EQ("", url.scheme());
  EXPECT_EQ("", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("/path/to", url.path());
  EXPECT_EQ("", url.query());
}

TEST(UrlTest, NoScheme2) {
  webcc::Url url("/path/to?key=value");

  EXPECT_EQ("", url.scheme());
  EXPECT_EQ("", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("/path/to", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(UrlTest, Full) {
  webcc::Url url("https://localhost:3000/path/to?key=value");

  EXPECT_EQ("https", url.scheme());
  EXPECT_EQ("localhost", url.host());
  EXPECT_EQ("3000", url.port());
  EXPECT_EQ("/path/to", url.path());
  EXPECT_EQ("key=value", url.query());
}
