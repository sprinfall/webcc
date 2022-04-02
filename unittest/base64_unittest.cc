#include "gtest/gtest.h"

#include "webcc/base64.h"

TEST(Base64Test, Encode) {
  std::string encoded;

  encoded = webcc::Base64Encode("ban");
  EXPECT_EQ(encoded, "YmFu");

  encoded = webcc::Base64Encode("bana");
  EXPECT_EQ(encoded, "YmFuYQ==");

  encoded = webcc::Base64Encode("banan");
  EXPECT_EQ(encoded, "YmFuYW4=");
}

TEST(Base64Test, Decode) {
  std::string decoded;

  decoded = webcc::Base64Decode("YmFu");
  EXPECT_EQ(decoded, "ban");

  decoded = webcc::Base64Decode("YmFuYQ==");
  EXPECT_EQ(decoded, "bana");

  decoded = webcc::Base64Decode("YmFuYW4=");
  EXPECT_EQ(decoded, "banan");
}
