#ifndef CSOAP_HTTP_REQUEST_PARSER_H_
#define CSOAP_HTTP_REQUEST_PARSER_H_

#include "csoap/http_parser.h"

namespace csoap {

class HttpRequest;

class HttpRequestParser : public HttpParser {
public:
  explicit HttpRequestParser(HttpRequest* request);

private:
  ErrorCode ParseStartLine(const std::string& line) override;

private:
  HttpRequest* request_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_PARSER_H_
