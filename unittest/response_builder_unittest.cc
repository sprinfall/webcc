#include "gtest/gtest.h"

#include "webcc/response_builder.h"

// Empty body should also have a Content-Length header.
TEST(ResponseBuilderTest, EmptyBody) {
  using namespace webcc;

  auto response = ResponseBuilder{}.OK()();

  std::string_view value = response->GetHeader(headers::kContentLength);

  EXPECT_EQ(value, "0");
}
