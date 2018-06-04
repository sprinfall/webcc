#include "webcc/http_message.h"

#include <sstream>

#include "boost/algorithm/string.hpp"

namespace webcc {

void HttpMessage::SetHeader(const std::string& name, const std::string& value) {
  for (HttpHeader& h : headers_) {
    if (h.name == name) {
      h.value = value;
      return;
    }
  }

  headers_.push_back({ name, value });
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

  os << std::endl;

  if (!content_.empty()) {
    if (indent == 0) {
      os << content_ << std::endl;
    } else {
      std::vector<std::string> splitted;
      boost::split(splitted, content_, boost::is_any_of("\r\n"));
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
