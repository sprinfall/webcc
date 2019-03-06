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

std::ostream& operator<<(std::ostream& os, const HttpMessage& message) {
  message.Dump(os);
  return os;
}

// -----------------------------------------------------------------------------

void HttpHeaderDict::Add(const std::string& key, const std::string& value) {
  for (HttpHeader& h : headers_) {
    if (boost::iequals(h.first, key)) {
      h.second = value;
      return;
    }
  }
  headers_.push_back({ key, value });
}

void HttpHeaderDict::Add(std::string&& key, std::string&& value) {
  for (HttpHeader& h : headers_) {
    if (boost::iequals(h.first, key)) {
      h.second = std::move(value);
      return;
    }
  }
  headers_.push_back({ std::move(key), std::move(value) });
}

bool HttpHeaderDict::Has(const std::string& key) const {
  for (const HttpHeader& h : headers_) {
    if (boost::iequals(h.first, key)) {
      return true;
    }
  }
  return false;
}

// -----------------------------------------------------------------------------

// See: https://tools.ietf.org/html/rfc7231#section-3.1.1.1
void HttpMessage::SetContentType(const std::string& media_type,
                                 const std::string& charset) {
  if (charset.empty()) {
    SetHeader(http::headers::kContentType, media_type);
  } else {
    SetHeader(http::headers::kContentType,
              media_type + ";charset=" + charset);
  }
}

void HttpMessage::SetContent(std::string&& content, bool set_length) {
  content_ = std::move(content);
  if (set_length) {
    SetContentLength(content_.size());
  }
}

void HttpMessage::SetContentInAppJsonUtf8(std::string&& content,
                                          bool set_length) {
  SetContent(std::move(content), set_length);
  SetContentType(http::media_types::kApplicationJson, http::charsets::kUtf8);
}

// ATTENTION: The buffers don't hold the memory!
std::vector<boost::asio::const_buffer> HttpMessage::ToBuffers() const {
  assert(!start_line_.empty());

  std::vector<boost::asio::const_buffer> buffers;

  buffers.push_back(boost::asio::buffer(start_line_));

  for (const HttpHeader& h : headers_.data()) {
    buffers.push_back(boost::asio::buffer(h.first));
    buffers.push_back(boost::asio::buffer(misc_strings::NAME_VALUE_SEPARATOR));
    buffers.push_back(boost::asio::buffer(h.second));
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

  for (const HttpHeader& h : headers_.data()) {
    os << indent_str << h.first << ": " << h.second << std::endl;
  }

  os << indent_str << std::endl;

  // NOTE: The content will be truncated if it's too large to display.

  if (!content_.empty()) {
    if (indent == 0) {
      if (content_.size() > kMaxDumpSize) {
        os.write(content_.c_str(), kMaxDumpSize);
        os << "..." << std::endl;
      } else {
        os << content_ << std::endl;
      }
    } else {
      // Split by EOL to achieve more readability.
      std::vector<std::string> splitted;
      boost::split(splitted, content_, boost::is_any_of(CRLF));

      std::size_t size = 0;

      for (const std::string& line : splitted) {
        os << indent_str;

        if (line.size() + size > kMaxDumpSize) {
          os.write(line.c_str(), kMaxDumpSize - size);
          os << "..." << std::endl;
          break;
        } else {
          os << line << std::endl;
          size += line.size();
        }
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

}  // namespace webcc
