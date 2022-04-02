#include "gtest/gtest.h"

#include "webcc/base64.h"

TEST(Base64Test, Encode) {
  using webcc::base64::Encode;

  EXPECT_EQ(Encode("ban"), "YmFu");
  EXPECT_EQ(Encode("bana"), "YmFuYQ==");
  EXPECT_EQ(Encode("banan"), "YmFuYW4=");
}

TEST(Base64Test, Decode) {
  using webcc::base64::Decode;

  EXPECT_EQ(Decode("YmFu"), "ban");
  EXPECT_EQ(Decode("YmFuYQ=="), "bana");
  EXPECT_EQ(Decode("YmFuYW4="), "banan");
}
