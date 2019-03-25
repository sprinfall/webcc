#ifndef WEBCC_HTTP_REQUEST_PARSER_H_
#define WEBCC_HTTP_REQUEST_PARSER_H_

#include <string>

#include "webcc/http_parser.h"

namespace webcc {

class HttpRequest;

class HttpRequestParser : public HttpParser {
public:
  explicit HttpRequestParser(HttpRequest* request = nullptr);

  ~HttpRequestParser() override = default;

  void Init(HttpRequest* request);

private:
  bool ParseStartLine(const std::string& line) final;

  HttpRequest* request_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_PARSER_H_
