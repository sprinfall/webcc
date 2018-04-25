#ifndef WEBCC_HTTP_REQUEST_PARSER_H_
#define WEBCC_HTTP_REQUEST_PARSER_H_

#include "webcc/http_parser.h"

namespace webcc {

class HttpRequest;

class HttpRequestParser : public HttpParser {
public:
  explicit HttpRequestParser(HttpRequest* request);

  ~HttpRequestParser() override = default;

private:
  Error ParseStartLine(const std::string& line) override;

private:
  HttpRequest* request_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_PARSER_H_
