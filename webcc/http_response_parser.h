#ifndef WEBCC_HTTP_RESPONSE_PARSER_H_
#define WEBCC_HTTP_RESPONSE_PARSER_H_

#include <string>

#include "webcc/http_parser.h"

namespace webcc {

class HttpResponse;

class HttpResponseParser : public HttpParser {
public:
  explicit HttpResponseParser(HttpResponse* response);

  ~HttpResponseParser() override = default;

private:
  // Parse HTTP start line; E.g., "HTTP/1.1 200 OK".
  bool ParseStartLine(const std::string& line) override;

  // The result response message.
  HttpResponse* response_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_RESPONSE_PARSER_H_
