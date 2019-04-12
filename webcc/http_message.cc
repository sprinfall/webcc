#include "webcc/http_message.h"

#include <sstream>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

const char HEADER_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const HttpMessage& message) {
  message.Dump(os);
  return os;
}

// -----------------------------------------------------------------------------

bool HttpMessage::IsConnectionKeepAlive() const {
  bool existed = false;
  const std::string& connection =
      GetHeader(http::headers::kConnection, &existed);

  if (!existed) {
    // Keep-Alive is by default for HTTP/1.1.
    return true;
  }

  if (boost::iequals(connection, "Keep-Alive")) {
    return true;
  }

  return false;
}

http::ContentEncoding HttpMessage::GetContentEncoding() const {
  const std::string& encoding = GetHeader(http::headers::kContentEncoding);
  if (encoding == "gzip") {
    return http::ContentEncoding::kGzip;
  }
  if (encoding == "deflate") {
    return http::ContentEncoding::kDeflate;
  }
  return http::ContentEncoding::kUnknown;
}

bool HttpMessage::AcceptEncodingGzip() const {
  using http::headers::kAcceptEncoding;

  return GetHeader(kAcceptEncoding).find("gzip") != std::string::npos;
}

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

void HttpMessage::Prepare() {
  assert(!start_line_.empty());

  using boost::asio::buffer;

  payload_.clear();

  payload_.push_back(buffer(start_line_));
  payload_.push_back(buffer(misc_strings::CRLF));

  for (const HttpHeader& h : headers_.data()) {
    payload_.push_back(buffer(h.first));
    payload_.push_back(buffer(misc_strings::HEADER_SEPARATOR));
    payload_.push_back(buffer(h.second));
    payload_.push_back(buffer(misc_strings::CRLF));
  }

  payload_.push_back(buffer(misc_strings::CRLF));

  if (!content_.empty()) {
    payload_.push_back(buffer(content_));
  }
}

void HttpMessage::Dump(std::ostream& os, std::size_t indent,
                       const std::string& prefix) const {
  std::string indent_str;
  if (indent > 0) {
    indent_str.append(indent, ' ');
  }
  indent_str.append(prefix);

  os << indent_str << start_line_ << std::endl;

  for (const HttpHeader& h : headers_.data()) {
    os << indent_str << h.first << ": " << h.second << std::endl;
  }

  os << indent_str << std::endl;

  // NOTE:
  // - The content will be truncated if it's too large to display.
  // - Binary content will not be dumped (TODO).

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
      std::vector<std::string> lines;
      boost::split(lines, content_, boost::is_any_of("\n"));

      std::size_t size = 0;

      for (const std::string& line : lines) {
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
