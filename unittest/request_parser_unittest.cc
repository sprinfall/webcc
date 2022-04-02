#include "gtest/gtest.h"

#include "webcc/request.h"
#include "webcc/request_parser.h"

// -----------------------------------------------------------------------------

bool ViewMatcher(const std::string& method, const std::string& url_path,
                 bool* stream) {
  *stream = false;
  return true;
}

// -----------------------------------------------------------------------------

// HTTP GET request parser test fixture.
class GetRequestParserTest : public testing::Test {
protected:
  void SetUp() override {
    payload_ =
        "GET /get HTTP/1.1\r\n"
        "Accept: application/json\r\n"
        "Connection: Close\r\n"
        "Host: httpbin.org\r\n\r\n";
  }

  void CheckResult(const webcc::Request& request) {
    EXPECT_EQ(request.method(), "GET");
    EXPECT_EQ(request.GetHeader("Host"), "httpbin.org");
    EXPECT_EQ(request.GetHeader("Accept"), "application/json");
    EXPECT_EQ(request.GetHeader("Connection"), "Close");

    EXPECT_EQ(request.data(), "");
    EXPECT_EQ(request.content_length(), webcc::kInvalidLength);
  }

  std::string payload_;
  webcc::RequestParser parser_;
};

// Parse once with all data.
TEST_F(GetRequestParserTest, ParseOnce) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

// Parse byte by byte.
TEST_F(GetRequestParserTest, ParseByteWise) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    ASSERT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

// Parse line by line.
TEST_F(GetRequestParserTest, ParseLineWise) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  for (std::size_t i = 0; i < payload_.size();) {
    std::size_t j = payload_.find('\n', i);

    if (j != std::string::npos) {
      bool ok = parser_.Parse(payload_.data() + i, j - i + 1);
      ASSERT_TRUE(ok);
    } else {
      break;
    }

    i = j + 1;
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

// Parse two requests using the same parser.
TEST_F(GetRequestParserTest, ParseTwoRounds) {
  // 1st round

  webcc::Request request1;
  parser_.Init(&request1, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request1);

  // 2nd round

  webcc::Request request2;
  parser_.Init(&request2, ViewMatcher);

  ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request2);
}

// -----------------------------------------------------------------------------

// HTTP POST request parser test fixture.
class PostRequestParserTest : public testing::Test {
protected:
  void SetUp() override {
    // clang-format off
    data_ =
        "{\n"
        "  'note': 'Webcc test',\n"
        "  'scopes': ['public_repo', 'repo', 'repo:status', 'user']\n"
        "}";

    payload_ =
        "POST /authorizations HTTP/1.1\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(data_.size()) + "\r\n"
        "Accept: application/json\r\n"
        "Connection: Close\r\n"
        "Host: api.github.com\r\n\r\n";
    // clang-format on

    payload_ += data_;
  }

  void CheckResult(const webcc::Request& request) {
    EXPECT_EQ(request.method(), "POST");
    EXPECT_EQ(request.GetHeader("Host"), "api.github.com");
    EXPECT_EQ(request.GetHeader("Accept"), "application/json");
    EXPECT_EQ(request.GetHeader("Connection"), "Close");
    EXPECT_EQ(request.GetHeader("Content-Type"),
              "application/json; charset=utf-8");
    EXPECT_EQ(request.GetHeader("Content-Length"),
              std::to_string(data_.size()));

    EXPECT_EQ(request.data(), data_);
    EXPECT_EQ(request.content_length(), data_.size());
  }

  std::string payload_;
  std::string data_;
  webcc::RequestParser parser_;
};

TEST_F(PostRequestParserTest, ParseOnce) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

TEST_F(PostRequestParserTest, ParseByteWise) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    ASSERT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

TEST_F(PostRequestParserTest, ParseTwoRounds) {
  // 1st round

  webcc::Request request1;
  parser_.Init(&request1, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request1);

  // 2nd round

  webcc::Request request2;
  parser_.Init(&request2, ViewMatcher);

  ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request2);
}

// -----------------------------------------------------------------------------

// HTTP multipart form request parser test fixture.
class MultipartRequestParserTest : public testing::Test {
protected:
  void SetUp() override {
    // clang-format off
    data_ =
        "--e81381de-436b-4314-8662-7362d5593b12\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"remember.txt\"\r\n"
        "\r\n"
        "Remember\r\n"
        "BY CHRISTINA ROSSETTI\r\n"
        "\r\n"
        "Remember me when I am gone away,\r\n"
        "         Gone far away into the silent land;\r\n"
        "         When you can no more hold me by the hand,\r\n"
        "Nor I half turn to go yet turning stay.\r\n"
        "Remember me when no more day by day\r\n"
        "         You tell me of our future that you plann'd:\r\n"
        "         Only remember me; you understand\r\n"
        "It will be late to counsel then or pray.\r\n"
        "Yet if you should forget me for a while\r\n"
        "         And afterwards remember, do not grieve:\r\n"
        "         For if the darkness and corruption leave\r\n"
        "         A vestige of the thoughts that once I had,\r\n"
        "Better by far you should forget and smile\r\n"
        "         Than that you should remember and be sad.\r\n"
        "\r\n"
        "--e81381de-436b-4314-8662-7362d5593b12\r\n"
        "Content-Disposition: form-data; name=\"json\"\r\n"
        "Content-Type: application/json\r\n"
        "\r\n"
        "{}\r\n"
        "--e81381de-436b-4314-8662-7362d5593b12--\r\n";

    payload_ =
        "POST / HTTP/1.1\r\n"
        "User-Agent: Webcc/0.1.0\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept: */*\r\n"
        "Connection: Keep-Alive\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: multipart/form-data; boundary=e81381de-436b-4314-8662-7362d5593b12\r\n"
        "Content-Length: " + std::to_string(data_.size()) + "\r\n"
        "\r\n";
    // clang-format on

    payload_ += data_;
  }

  void CheckResult(const webcc::Request& request) {
    EXPECT_EQ(request.method(), "POST");
    EXPECT_EQ(request.GetHeader("Host"), "localhost:8080");
    EXPECT_EQ(request.GetHeader("Accept"), "*/*");
    EXPECT_EQ(request.GetHeader("Connection"), "Keep-Alive");
    EXPECT_EQ(request.form_parts().size(), 2);
  }

  std::string payload_;
  std::string data_;
  webcc::RequestParser parser_;
};

TEST_F(MultipartRequestParserTest, ParseOnce) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

TEST_F(MultipartRequestParserTest, ParseByteWise) {
  webcc::Request request;
  parser_.Init(&request, ViewMatcher);

  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    ASSERT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult(request);
}

TEST_F(MultipartRequestParserTest, ParseTwoRounds) {
  // 1st round

  webcc::Request request1;
  parser_.Init(&request1, ViewMatcher);

  bool ok = parser_.Parse(payload_.data(), payload_.size());

  ASSERT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request1);

  // 2nd round

  webcc::Request request2;
  parser_.Init(&request2, ViewMatcher);

  ok = parser_.Parse(payload_.data(), payload_.size());

  EXPECT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult(request2);
}
