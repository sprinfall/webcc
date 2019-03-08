#include "webcc/http_request.h"

#include "webcc/logger.h"

namespace webcc {

bool HttpRequest::Prepare() {
  if (url_.host().empty()) {
    LOG_ERRO("Invalid request: host is missing.");
    return false;
  }

  std::string target = "/" + url_.path();
  if (!url_.query().empty()) {
    target += "?";
    target += url_.query();
  }

  start_line_ = method_;
  start_line_ += " ";
  start_line_ += target;
  start_line_ += " HTTP/1.1";
  start_line_ += kCRLF;

  if (url_.port().empty()) {
    SetHeader(http::headers::kHost, url_.host());
  } else {
    SetHeader(http::headers::kHost, url_.host() + ":" + url_.port());
  }

  return true;
}

}  // namespace webcc
