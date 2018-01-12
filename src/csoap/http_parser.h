#ifndef CSOAP_HTTP_PARSER_H_
#define CSOAP_HTTP_PARSER_H_

#include <string>

#include "csoap/common.h"

namespace csoap {

class HttpMessage;

// HttpParser parses HTTP request and response.
class HttpParser {
public:
  explicit HttpParser(HttpMessage* message);

  bool finished() const {
    return finished_;
  }

  // Reset parsing state.
  void Reset();

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

}  // namespace csoap

#endif  // CSOAP_HTTP_PARSER_H_
