#ifndef WEBCC_HTTP_PARSER_H_
#define WEBCC_HTTP_PARSER_H_

#include <string>
#include "webcc/common.h"

namespace webcc {

class HttpMessage;

// HttpParser parses HTTP request and response.
class HttpParser {
public:
  explicit HttpParser(HttpMessage* message);

  bool finished() const {
    return finished_;
  }

  Error Parse(const char* data, size_t len);

protected:
  // Parse HTTP start line.
  virtual Error ParseStartLine(const std::string& line) = 0;

  void ParseContentLength(const std::string& line);

protected:
  // The result HTTP message.
  HttpMessage* message_;

  Error error_;

  // Data waiting to be parsed.
  std::string pending_data_;

  // Parsing helper flags.
  bool start_line_parsed_;
  bool header_parsed_;
  bool finished_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_PARSER_H_
