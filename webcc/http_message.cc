#include "webcc/http_message.h"

#include <sstream>

#include "boost/algorithm/string.hpp"

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

const char NAME_VALUE_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// -----------------------------------------------------------------------------

void HttpMessage::SetHeader(const std::string& name, const std::string& value) {
  for (HttpHeader& h : headers_) {
    if (boost::iequals(h.name, name)) {
      h.value = value;
      return;
    }
  }
  headers_.push_back({ name, value });
}

void HttpMessage::SetHeader(std::string&& name, std::string&& value) {
  for (HttpHeader& h : headers_) {
    if (boost::iequals(h.name, name)) {
      h.value = std::move(value);
      return;
    }
  }
  headers_.push_back({ std::move(name), std::move(value) });
}

// ATTENTION: The buffers don't hold the memory!
std::vector<boost::asio::const_buffer> HttpMessage::ToBuffers() const {
  assert(!start_line_.empty());

  std::vector<boost::asio::const_buffer> buffers;

  buffers.push_back(boost::asio::buffer(start_line_));

  for (const HttpHeader& h : headers_) {
    buffers.push_back(boost::asio::buffer(h.name));
    buffers.push_back(boost::asio::buffer(misc_strings::NAME_VALUE_SEPARATOR));
    buffers.push_back(boost::asio::buffer(h.value));
    buffers.push_back(boost::asio::buffer(misc_strings::CRLF));
  }

  buffers.push_back(boost::asio::buffer(misc_strings::CRLF));

  if (content_length_ > 0) {
    buffers.push_back(boost::asio::buffer(content_));
  }

  return buffers;
}

void HttpMessage::Dump(std::ostream& os, std::size_t indent,
                       const std::string& prefix) const {
  std::string indent_str;
  if (indent > 0) {
    indent_str.append(indent, ' ');
  }
  indent_str.append(prefix);

  os << indent_str << start_line_;

  for (const HttpHeader& h : headers_) {
    os << indent_str << h.name << ": " << h.value << std::endl;
  }

  os << indent_str << std::endl;

  if (!content_.empty()) {
    if (indent == 0) {
      os << content_ << std::endl;
    } else {
      std::vector<std::string> splitted;
      boost::split(splitted, content_, boost::is_any_of(CRLF));
      for (const std::string& line : splitted) {
        os << indent_str << line << std::endl;
      }
    }
  }
}

std::string HttpMessage::Dump(std::size_t indent,
                              const std::string& prefix) const {
  std::stringstream ss;
  Dump(ss, indent, prefix);
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HttpMessage& message) {
  message.Dump(os);
  return os;
}

}  // namespace webcc
