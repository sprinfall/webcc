#ifndef CSOAP_HTTP_MESSAGE_H_
#define CSOAP_HTTP_MESSAGE_H_

#include <cassert>
#include <string>
#include "csoap/common.h"

namespace csoap {

class HttpHeader {
public:
  std::string name;
  std::string value;
};

// Base class for HTTP request and response messages.
class HttpMessage {
public:
  void set_version(HttpVersion version) {
    version_ = version;
  }

  size_t content_length() const {
    return content_length_;
  }
  void set_content_length(size_t length) {
    content_length_ = length;
  }

  // E.g., "text/xml; charset=utf-8"
  void set_content_type(const std::string& content_type) {
    content_type_ = content_type;
  }

  void AddHeader(const std::string& name, const std::string& value) {
    headers_.push_back({ name, value });
  }

  const std::string& content() const {
    return content_;
  }
  void set_content(const std::string& content) {
    content_ = content;
  }

  void AppendContent(const char* data, size_t count) {
    content_.append(data, count);
  }

  void AppendContent(const std::string& data) {
    content_.append(data);
  }

  bool IsContentFull() const {
    assert(content_length_ != kInvalidLength);
    return content_.length() >= content_length_;
  }

protected:
  HttpMessage() {
  }

protected:
  HttpVersion version_ = kHttpV11;

  size_t content_length_ = kInvalidLength;
  std::string content_type_;

  std::vector<HttpHeader> headers_;

  std::string content_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_MESSAGE_H_
