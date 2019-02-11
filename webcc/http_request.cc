#include "webcc/http_request.h"

namespace webcc {

HttpRequest::HttpRequest(const std::string& method,
                         const std::string& url,
                         const std::string& host,
                         const std::string& port)
    : method_(method), url_(url), host_(host), port_(port) {
}

void HttpRequest::Make() {
  start_line_ = method_;
  start_line_ += " ";
  start_line_ += url_;
  start_line_ += " HTTP/1.1";
  start_line_ += CRLF;

  if (port_.empty()) {
    SetHeader(http::headers::kHost, host_);
  } else {
    SetHeader(http::headers::kHost, host_ + ":" + port_);
  }

  // TODO: Support Keep-Alive connection.
  //SetHeader(http::headers::kConnection, "close");

  // TODO: Support gzip, deflate
  SetHeader(http::headers::kAcceptEncoding, "identity");

  // NOTE: C++11 requires a space between literal and string macro.
  SetHeader(http::headers::kUserAgent, "Webcc/" WEBCC_VERSION);
}

}  // namespace webcc
