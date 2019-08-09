#include "gtest/gtest.h"

#include "webcc/response_builder.h"

// Empty body should also have `Content-Length` header.
TEST(ResponseBuilderTest, EmptyBody) {
  using namespace webcc;

  auto response = ResponseBuilder{}.OK()();

  bool existed = false;
  const auto& value = response->GetHeader(headers::kContentLength, &existed);

  EXPECT_TRUE(existed);

  EXPECT_EQ("0", value);
}
