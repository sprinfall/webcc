#include "gtest/gtest.h"

#include "webcc/body.h"

TEST(FormBodyTest, Payload) {
  std::vector<webcc::FormPartPtr> parts{
    webcc::FormPart::New("json", "{}", "application/json")
  };

  webcc::FormBody form_body{ parts, "123456" };

  form_body.InitPayload();

  webcc::Payload payload = form_body.NextPayload();
  EXPECT_FALSE(payload.empty());

  payload = form_body.NextPayload();
  EXPECT_TRUE(payload.empty());
}
