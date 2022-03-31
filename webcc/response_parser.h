#ifndef WEBCC_RESPONSE_PARSER_H_
#define WEBCC_RESPONSE_PARSER_H_

#include <string>

#include "webcc/parser.h"

namespace webcc {

class Response;

class ResponseParser : public Parser {
public:
  ResponseParser() = default;
  ~ResponseParser() override = default;

  void Init(Response* response, bool stream = false);

  void set_ignore_body(bool ignore_body) {
    ignore_body_ = ignore_body;
  }

private:
  bool OnHeadersEnd() override {
    return true;
  }

  // Parse HTTP start line; E.g., "HTTP/1.1 200 OK".
  bool ParseStartLine(const std::string& line) override;

  // Override to allow to ignore the body of the response for HEAD request.
  bool ParseContent(const char* data, std::size_t length) override;

private:
  // The result response message.
  Response* response_ = nullptr;

  // The response for HEAD request could also have a Content-Length header.
  // Set this flag to ignore it.
  bool ignore_body_ = false;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_PARSER_H_
