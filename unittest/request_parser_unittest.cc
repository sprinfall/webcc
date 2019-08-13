#include "gtest/gtest.h"

#include "webcc/request.h"
#include "webcc/request_parser.h"

// -----------------------------------------------------------------------------

#if 0
// HTTP GET request parser test fixture.
class GetRequestParserTest : public testing::Test {
protected:
  GetRequestParserTest() {
    parser_.Init(&request_);
  }

  void SetUp() override {
    payload_ =
      "GET /get HTTP/1.1\r\n"
      "Accept: application/json\r\n"
      "Connection: Close\r\n"
      "Host: httpbin.org\r\n\r\n";
  }

  void CheckResult() {
    EXPECT_EQ("GET", request_.method());
    EXPECT_EQ("httpbin.org", request_.GetHeader("Host"));
    EXPECT_EQ("application/json", request_.GetHeader("Accept"));
    EXPECT_EQ("Close", request_.GetHeader("Connection"));

    EXPECT_EQ("", request_.data());
    EXPECT_EQ(webcc::kInvalidLength, request_.content_length());
  }

  std::string payload_;

  webcc::Request request_;
  webcc::RequestParser parser_;
};

TEST_F(GetRequestParserTest, ParseOnce) {
  bool ok = parser_.Parse(payload_.data(), payload_.size());

  EXPECT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

// Parse byte by byte.
TEST_F(GetRequestParserTest, ParseByteWise) {
  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    EXPECT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

// Parse line by line.
TEST_F(GetRequestParserTest, ParseLineWise) {
  for (std::size_t i = 0; i < payload_.size();) {
    std::size_t j = payload_.find('\n', i);

    if (j != std::string::npos) {
      bool ok = parser_.Parse(payload_.data() + i, j - i + 1);
      EXPECT_TRUE(ok);
    } else {
      break;
    }

    i = j + 1;
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

// -----------------------------------------------------------------------------

// HTTP POST request parser test fixture.
class PostRequestParserTest : public testing::Test {
protected:
  PostRequestParserTest() {
    parser_.Init(&request_);
  }

  void SetUp() override {
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

    payload_ += data_;
  }

  void CheckResult() {
    EXPECT_EQ("POST", request_.method());
    EXPECT_EQ("api.github.com", request_.GetHeader("Host"));
    EXPECT_EQ("application/json", request_.GetHeader("Accept"));
    EXPECT_EQ("Close", request_.GetHeader("Connection"));
    EXPECT_EQ("application/json; charset=utf-8", request_.GetHeader("Content-Type"));
    EXPECT_EQ(std::to_string(data_.size()), request_.GetHeader("Content-Length"));

    EXPECT_EQ(data_, request_.data());
    EXPECT_EQ(data_.size(), request_.content_length());
  }

  std::string payload_;
  std::string data_;

  webcc::Request request_;
  webcc::RequestParser parser_;
};

TEST_F(PostRequestParserTest, ParseOnce) {
  bool ok = parser_.Parse(payload_.data(), payload_.size());

  EXPECT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

// Parse byte by byte.
TEST_F(PostRequestParserTest, ParseByteWise) {
  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    EXPECT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

// -----------------------------------------------------------------------------

class MultipartRequestParserTest : public testing::Test {
protected:
  MultipartRequestParserTest() {
    parser_.Init(&request_);
  }

  void SetUp() override {
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

    payload_ += data_;
  }

  void CheckResult() {
    EXPECT_EQ("POST", request_.method());
    EXPECT_EQ("localhost:8080", request_.GetHeader("Host"));
    EXPECT_EQ("*/*", request_.GetHeader("Accept"));
    EXPECT_EQ("Keep-Alive", request_.GetHeader("Connection"));

    EXPECT_EQ(2, request_.form_parts().size());
  }

  std::string payload_;
  std::string data_;

  webcc::Request request_;
  webcc::RequestParser parser_;
};

TEST_F(MultipartRequestParserTest, ParseOnce) {
  bool ok = parser_.Parse(payload_.data(), payload_.size());

  EXPECT_TRUE(ok);
  EXPECT_TRUE(parser_.finished());

  CheckResult();
}

TEST_F(MultipartRequestParserTest, ParseByteWise) {
  for (std::size_t i = 0; i < payload_.size(); ++i) {
    bool ok = parser_.Parse(payload_.data() + i, 1);
    EXPECT_TRUE(ok);
  }

  EXPECT_TRUE(parser_.finished());

  CheckResult();
}
#endif  // 0