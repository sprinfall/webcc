#ifndef WEBCC_MESSAGE_H_
#define WEBCC_MESSAGE_H_

#include <cassert>
#include <string>
#include <utility>  // for move()
#include <vector>

#include "webcc/common.h"
#include "webcc/globals.h"

namespace webcc {

class Message;
std::ostream& operator<<(std::ostream& os, const Message& message);

// Base class for HTTP request and response messages.
class Message {
public:
  Message() : content_length_(kInvalidLength) {
  }

  virtual ~Message() = default;

  const std::string& start_line() const {
    return start_line_;
  }

  void set_start_line(const std::string& start_line) {
    start_line_ = start_line;
  }

  std::size_t content_length() const {
    return content_length_;
  }

  void set_content_length(std::size_t content_length) {
    content_length_ = content_length;
  }

  const std::string& content() const {
    return content_;
  }

  bool IsConnectionKeepAlive() const;

  void SetHeader(Header&& header) {
    headers_.Set(std::move(header.first), std::move(header.second));
  }

  void SetHeader(const std::string& key, const std::string& value) {
    headers_.Set(key, value);
  }

  const std::string& GetHeader(const std::string& key,
                               bool* existed = nullptr) const {
    return headers_.Get(key, existed);
  }

  bool HasHeader(const std::string& key) const {
    return headers_.Has(key);
  }

  ContentEncoding GetContentEncoding() const;

  // Return true if header Accept-Encoding contains "gzip".
  bool AcceptEncodingGzip() const;

  const ContentType& content_type() const {
    return content_type_;
  }

  // TODO: Set header?
  void SetContentType(const ContentType& content_type) {
    content_type_ = content_type;
  }

  void SetContentType(const std::string& content_type) {
    SetHeader(headers::kContentType, content_type);
  }

  // Example: SetContentType("application/json", "utf-8")
  void SetContentType(const std::string& media_type,
                      const std::string& charset);

  void SetContent(std::string&& content, bool set_length);

  // Prepare payload.
  virtual void Prepare();

  const Payload& payload() const {
    return payload_;
  }

  // Copy the exact payload to the given output stream.
  void CopyPayload(std::ostream& os) const;

  // Copy the exact payload to the given string.
  void CopyPayload(std::string* str) const;

  // Dump to output stream.
  void Dump(std::ostream& os, std::size_t indent = 0,
            const std::string& prefix = "") const;

  // Dump to string, only used by logger.
  std::string Dump(std::size_t indent = 0,
                   const std::string& prefix = "") const;

protected:
  void SetContentLength(std::size_t content_length) {
    content_length_ = content_length;
    SetHeader(headers::kContentLength, std::to_string(content_length));
  }

protected:
  std::string start_line_;

  std::string content_;

  ContentType content_type_;

  std::size_t content_length_;

  Headers headers_;

  // NOTE: The payload itself doesn't hold the memory!
  Payload payload_;
};

}  // namespace webcc

#endif  // WEBCC_MESSAGE_H_
