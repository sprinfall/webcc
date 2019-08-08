#ifndef WEBCC_RESPONSE_PARSER_H_
#define WEBCC_RESPONSE_PARSER_H_

#include <string>

#include "webcc/parser.h"

namespace webcc {

class Response;

class ResponseParser : public Parser {
public:
  ResponseParser();

  ~ResponseParser() override = default;

  void Init(Response* response, bool stream = false);

  void set_ignroe_body(bool ignroe_body) {
    ignroe_body_ = ignroe_body;
  }

private:
  // Parse HTTP start line; E.g., "HTTP/1.1 200 OK".
  bool ParseStartLine(const std::string& line) override;

  // Override to allow to ignore the body of the response for HEAD request.
  bool ParseContent(const char* data, std::size_t length) override;

private:
  // The result response message.
  Response* response_;

  // The response for HEAD request could also have `Content-Length` header,
  // set this flag to ignore it.
  bool ignroe_body_ = false;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_PARSER_H_
