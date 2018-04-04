#include "csoap/http_request.h"

#include "boost/algorithm/string.hpp"

namespace csoap {

std::ostream& operator<<(std::ostream& os, const HttpRequest& request) {
  os << request.start_line();

  for (const HttpHeader& h : request.headers_) {
    os << h.name << ": " << h.value << std::endl;
  }

  os << std::endl;

  os << request.content() << std::endl;

  return os;
}

void HttpRequest::SetHost(const std::string& host, const std::string& port) {
  host_ = host;
  port_ = port;

  if (port.empty()) {
    SetHeader(kHost, host);
  } else {
    SetHeader(kHost, host + ":" + port);
  }
}

void HttpRequest::MakeStartLine() {
  if (start_line_.empty()) {
    start_line_ = method_;
    start_line_ += " ";
    start_line_ += url_;
    start_line_ += " HTTP/1.1\r\n";
  }
}

namespace misc_strings {

const char NAME_VALUE_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

// ATTENTION: The buffers don't hold the memory!
std::vector<boost::asio::const_buffer> HttpRequest::ToBuffers() const {
  assert(!start_line_.empty());
  assert(IsContentLengthValid());

  std::vector<boost::asio::const_buffer> buffers;

  buffers.push_back(boost::asio::buffer(start_line_));

  for (const HttpHeader& h : headers_) {
    buffers.push_back(boost::asio::buffer(h.name));
    buffers.push_back(boost::asio::buffer(misc_strings::NAME_VALUE_SEPARATOR));
    buffers.push_back(boost::asio::buffer(h.value));
    buffers.push_back(boost::asio::buffer(misc_strings::CRLF));
  }

  buffers.push_back(boost::asio::buffer(misc_strings::CRLF));

  buffers.push_back(boost::asio::buffer(content_));

  return buffers;
}

}  // namespace csoap
