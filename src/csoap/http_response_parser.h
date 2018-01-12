#ifndef CSOAP_HTTP_RESPONSE_PARSER_H_
#define CSOAP_HTTP_RESPONSE_PARSER_H_

#include "csoap/http_parser.h"

namespace csoap {

class HttpResponse;

class HttpResponseParser : public HttpParser {
public:
  explicit HttpResponseParser(HttpResponse* response);

private:
  // Parse HTTP start line; E.g., "HTTP/1.1 200 OK".
  Error ParseStartLine(const std::string& line) override;

private:
  // The result response message.
  HttpResponse* response_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_RESPONSE_PARSER_H_
