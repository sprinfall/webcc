#include "webcc/message.h"

#include <ctime>
#include <iomanip>  // for put_time
#include <sstream>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace misc_strings {

// Literal strings can't be used because they have an extra '\0'.

const char HEADER_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const Message& message) {
  message.Dump(os);
  return os;
}

// -----------------------------------------------------------------------------

bool Message::IsConnectionKeepAlive() const {
  using headers::kConnection;

  bool existed = false;
  const std::string& connection = GetHeader(kConnection, &existed);

  if (!existed) {
    // Keep-Alive is by default for HTTP/1.1.
    return true;
  }

  if (boost::iequals(connection, "Keep-Alive")) {
    return true;
  }

  return false;
}

ContentEncoding Message::GetContentEncoding() const {
  using headers::kContentEncoding;

  const std::string& encoding = GetHeader(kContentEncoding);
  if (encoding == "gzip") {
    return ContentEncoding::kGzip;
  }
  if (encoding == "deflate") {
    return ContentEncoding::kDeflate;
  }
  return ContentEncoding::kUnknown;
}

bool Message::AcceptEncodingGzip() const {
  using headers::kAcceptEncoding;

  return GetHeader(kAcceptEncoding).find("gzip") != std::string::npos;
}

// See: https://tools.ietf.org/html/rfc7231#section-3.1.1.1
void Message::SetContentType(const std::string& media_type,
                             const std::string& charset) {
  using headers::kContentType;

  if (charset.empty()) {
    SetHeader(kContentType, media_type);
  } else {
    SetHeader(kContentType, media_type + ";charset=" + charset);
  }
}

void Message::SetContent(std::string&& content, bool set_length) {
  content_ = std::move(content);
  if (set_length) {
    SetContentLength(content_.size());
  }
}

void Message::Prepare() {
  assert(!start_line_.empty());

  using boost::asio::buffer;

  payload_.clear();

  payload_.push_back(buffer(start_line_));
  payload_.push_back(buffer(misc_strings::CRLF));

  for (const Header& h : headers_.data()) {
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

void Message::CopyPayload(std::ostream& os) const {
  for (const boost::asio::const_buffer& b : payload_) {
    os.write(static_cast<const char*>(b.data()), b.size());
  }
}

void Message::CopyPayload(std::string* str) const {
  std::stringstream ss;
  CopyPayload(ss);
  *str = ss.str();
}

void Message::Dump(std::ostream& os, std::size_t indent,
                   const std::string& prefix) const {
  std::string indent_str;
  if (indent > 0) {
    indent_str.append(indent, ' ');
  }
  indent_str.append(prefix);

  os << indent_str << start_line_ << std::endl;

  for (const Header& h : headers_.data()) {
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

std::string Message::Dump(std::size_t indent,
                          const std::string& prefix) const {
  std::stringstream ss;
  Dump(ss, indent, prefix);
  return ss.str();
}

std::string Message::GetTimestamp() {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S") << " GMT";
  return ss.str();
}

}  // namespace webcc
