#ifndef WEBCC_HTTP_MESSAGE_H_
#define WEBCC_HTTP_MESSAGE_H_

#include <cassert>
#include <string>
#include <utility>  // for move()
#include <vector>

#include "boost/asio/buffer.hpp"  // for const_buffer

#include "webcc/globals.h"

namespace webcc {

struct HttpHeader {
  std::string name;
  std::string value;
};

// Base class for HTTP request and response messages.
class HttpMessage {
 public:
  HttpMessage() : content_length_(kInvalidLength) {}

  virtual ~HttpMessage() = default;

  const std::string& start_line() const { return start_line_; }

  void set_start_line(const std::string& start_line) {
    start_line_ = start_line;
  }

  std::size_t content_length() const { return content_length_; }

  const std::string& content() const { return content_; }

  void SetHeader(const std::string& name, const std::string& value);

  void SetHeader(std::string&& name, std::string&& value);

  // E.g., "application/json; charset=utf-8"
  void SetContentType(const std::string& content_type) {
    SetHeader(kContentType, content_type);
  }

  void SetContent(std::string&& content, bool set_length) {
    content_ = std::move(content);
    if (set_length) {
      SetContentLength(content_.size());
    }
  }

  // Set start line according to other informations.
  // Must be called before ToBuffers()!
  virtual void UpdateStartLine() = 0;

  // Convert the message into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the message object must remain valid
  // and not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> ToBuffers() const;

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

  // Start line with trailing CRLF.
  std::string start_line_;

  std::size_t content_length_;

  std::vector<HttpHeader> headers_;

  std::string content_;
};

std::ostream& operator<<(std::ostream& os, const HttpMessage& message);

}  // namespace webcc

#endif  // WEBCC_HTTP_MESSAGE_H_
