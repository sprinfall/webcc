#include "gtest/gtest.h"

#include "webcc/url.h"

TEST(Url, Basic) {
  webcc::Url url("http://example.com/path", false);

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("path", url.path());
  EXPECT_EQ("", url.query());
}

TEST(Url, NoPath) {
  webcc::Url url("http://example.com", false);

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("", url.query());
}

TEST(Url, NoPath2) {
  webcc::Url url("http://example.com/", false);

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("", url.query());
}

TEST(Url, NoPath3) {
  webcc::Url url("http://example.com?key=value", false);

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(Url, NoPath4) {
  webcc::Url url("http://example.com/?key=value", false);

  EXPECT_EQ("http", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(Url, NoScheme) {
  webcc::Url url("/path/to", false);

  EXPECT_EQ("", url.scheme());
  EXPECT_EQ("", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("path/to", url.path());
  EXPECT_EQ("", url.query());
}

TEST(Url, NoScheme2) {
  webcc::Url url("/path/to?key=value", false);

  EXPECT_EQ("", url.scheme());
  EXPECT_EQ("", url.host());
  EXPECT_EQ("", url.port());
  EXPECT_EQ("path/to", url.path());
  EXPECT_EQ("key=value", url.query());
}

TEST(Url, Full) {
  webcc::Url url("https://localhost:3000/path/to?key=value", false);

  EXPECT_EQ("https", url.scheme());
  EXPECT_EQ("localhost", url.host());
  EXPECT_EQ("3000", url.port());
  EXPECT_EQ("path/to", url.path());
  EXPECT_EQ("key=value", url.query());
}
