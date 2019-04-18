#ifndef WEBCC_RESPONSE_PARSER_H_
#define WEBCC_RESPONSE_PARSER_H_

#include <string>

#include "webcc/parser.h"

namespace webcc {

class Response;

class ResponseParser : public Parser {
public:
  explicit ResponseParser(Response* response = nullptr);

  ~ResponseParser() override = default;

  void Init(Response* response);

private:
  // Parse HTTP start line; E.g., "HTTP/1.1 200 OK".
  bool ParseStartLine(const std::string& line) final;

  // The result response message.
  Response* response_;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_PARSER_H_
