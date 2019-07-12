#include "gtest/gtest.h"

#include "webcc/body.h"

TEST(FormBodyTest, Payload) {
  std::vector<webcc::FormPartPtr> parts{
    std::make_shared<webcc::FormPart>("json", "{}", "application/json")
  };

  webcc::FormBody form_body{ parts, "123456" };

  form_body.InitPayload();

  auto payload = form_body.NextPayload();
  EXPECT_TRUE(!payload.empty());

  payload = form_body.NextPayload();
  EXPECT_TRUE(payload.empty());
}
