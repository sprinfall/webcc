#include "webcc/http_request.h"

namespace webcc {

void HttpRequest::SetHost(const std::string& host, const std::string& port) {
  host_ = host;
  port_ = port;

  if (port.empty()) {
    SetHeader(kHost, host);
  } else {
    SetHeader(kHost, host + ":" + port);
  }
}

void HttpRequest::UpdateStartLine() {
  start_line_ = method_;
  start_line_ += " ";
  start_line_ += url_;
  start_line_ += " HTTP/1.1";
  start_line_ += CRLF;
}

}  // namespace webcc
