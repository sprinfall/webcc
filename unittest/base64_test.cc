#include "gtest/gtest.h"

#include "webcc/base64.h"

TEST(Base64, Encode) {
  std::string encoded;

  encoded = webcc::Base64Encode("ban");
  EXPECT_EQ("YmFu", encoded);

  encoded = webcc::Base64Encode("bana");
  EXPECT_EQ("YmFuYQ==", encoded);

  encoded = webcc::Base64Encode("banan");
  EXPECT_EQ("YmFuYW4=", encoded);
}

TEST(Base64, Decode) {
  std::string decoded;

  decoded = webcc::Base64Decode("YmFu");
  EXPECT_EQ("ban", decoded);

  decoded = webcc::Base64Decode("YmFuYQ==");
  EXPECT_EQ("bana", decoded);

  decoded = webcc::Base64Decode("YmFuYW4=");
  EXPECT_EQ("banan", decoded);
}
