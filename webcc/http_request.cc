#include "webcc/http_request.h"

namespace webcc {

void HttpRequest::Make() {
  start_line_ = method_;
  start_line_ += " ";
  start_line_ += url_;
  start_line_ += " HTTP/1.1";
  start_line_ += CRLF;

  if (port_.empty()) {
    SetHeader(kHost, host_);
  } else {
    SetHeader(kHost, host_ + ":" + port_);
  }

  // NOTE: C++11 requires a space between literal and string macro.
  SetHeader(kUserAgent, "Webcc/" WEBCC_VERSION);
}

}  // namespace webcc
