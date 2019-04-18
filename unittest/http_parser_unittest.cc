#include "gtest/gtest.h"

#include "webcc/http_request.h"
#include "webcc/http_request_parser.h"
#include "webcc/http_response.h"
#include "webcc/http_response_parser.h"

#include <iostream>

// -----------------------------------------------------------------------------

// HTTP GET request parser test fixture.
class GetRequestParserTest : public testing::Test {
protected:
  GetRequestParserTest() : parser_(&request_) {
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

    EXPECT_EQ("", request_.content());
    EXPECT_EQ(0, request_.content_length());
  }

  std::string payload_;

  webcc::HttpRequest request_;
  webcc::HttpRequestParser parser_;
};

TEST_F(GetRequestParserTest, ParseFullDataOnce) {
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
  PostRequestParserTest() : parser_(&request_) {
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

    EXPECT_EQ(data_, request_.content());
    EXPECT_EQ(data_.size(), request_.content_length());
  }

  std::string payload_;
  std::string data_;

  webcc::HttpRequest request_;
  webcc::HttpRequestParser parser_;
};

TEST_F(PostRequestParserTest, ParseFullDataOnce) {
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
