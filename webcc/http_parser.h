#ifndef WEBCC_HTTP_PARSER_H_
#define WEBCC_HTTP_PARSER_H_

#include <string>

#include "webcc/globals.h"

namespace webcc {

class HttpMessage;

// HttpParser parses HTTP request and response.
class HttpParser {
 public:
  explicit HttpParser(HttpMessage* message);

  virtual ~HttpParser() = default;

  WEBCC_DELETE_COPY_ASSIGN(HttpParser);

  bool finished() const { return finished_; }

  bool content_length_parsed() const { return content_length_parsed_; }
  std::size_t content_length() const { return content_length_; }

  bool Parse(const char* data, std::size_t length);

 protected:
  virtual bool ParseStartLine(const std::string& line) = 0;

  bool ParseHeader(const std::string& line);

  void Finish();

  void AppendContent(const char* data, std::size_t count);
  void AppendContent(const std::string& data);

  bool IsContentFull() const;

  // The result HTTP message.
  HttpMessage* message_;

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
