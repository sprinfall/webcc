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

  ~HttpParser() = default;
  HttpParser(const HttpParser&) = delete;
  HttpParser& operator=(const HttpParser&) = delete;

  bool finished() const {
    return finished_;
  }

  Error Parse(const char* data, std::size_t len);

protected:
  // Parse HTTP start line.
  virtual Error ParseStartLine(const std::string& line) = 0;

  void ParseContentLength(const std::string& line);

  void Finish();

  void AppendContent(const char* data, std::size_t count);
  void AppendContent(const std::string& data);

  bool IsContentFull() const;

protected:
  // The result HTTP message.
  HttpMessage* message_;

  Error error_;

  // Data waiting to be parsed.
  std::string pending_data_;

  // Temporary data and helper flags for parsing.
  std::size_t content_length_;
  std::string content_;
  bool start_line_parsed_;
  bool content_length_parsed_;
  bool header_parsed_;
  bool finished_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_PARSER_H_
