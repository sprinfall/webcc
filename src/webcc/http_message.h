#ifndef WEBCC_HTTP_MESSAGE_H_
#define WEBCC_HTTP_MESSAGE_H_

#include <cassert>
#include <string>
#include <utility>  // for move()
#include <vector>

#include "webcc/globals.h"

namespace webcc {

struct HttpHeader {
  std::string name;
  std::string value;
};

// Base class for HTTP request and response messages.
class HttpMessage {
 public:
  virtual ~HttpMessage() = default;

  const std::string& start_line() const { return start_line_; }

  void set_start_line(const std::string& start_line) {
    start_line_ = start_line;
  }

  std::size_t content_length() const { return content_length_; }

  const std::string& content() const { return content_; }

  void SetHeader(const std::string& name, const std::string& value);

  // E.g., "text/xml; charset=utf-8"
  void SetContentType(const std::string& content_type) {
    SetHeader(kContentType, content_type);
  }

  void SetContent(std::string&& content) {
    content_ = std::move(content);
    SetContentLength(content_.size());
  }

  void SetContent(const std::string& content) {
    content_ = content;
    SetContentLength(content_.size());
  }

  // Dump to output stream.
  void Dump(std::ostream& os, std::size_t indent = 0,
            const std::string& prefix = "") const;

  // Dump to string, only used by logger.
  std::string Dump(std::size_t indent = 0,
                   const std::string& prefix = "") const;

 protected:
  void SetContentLength(std::size_t content_length) {
    content_length_ = content_length;
    SetHeader(kContentLength, std::to_string(content_length));
  }

  // Start line with trailing "\r\n".
  std::string start_line_;

  std::size_t content_length_ = kInvalidLength;

  std::vector<HttpHeader> headers_;

  std::string content_;
};

std::ostream& operator<<(std::ostream& os, const HttpMessage& message);

}  // namespace webcc

#endif  // WEBCC_HTTP_MESSAGE_H_
