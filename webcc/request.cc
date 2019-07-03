#include "webcc/request.h"

namespace webcc {

bool Request::IsForm() const {
  return !!std::dynamic_pointer_cast<FormBody>(body_);
}

const std::vector<FormPartPtr>& Request::form_parts() const {
  auto form_body = std::dynamic_pointer_cast<FormBody>(body_);

  if (!form_body) {
    throw Error{ Error::kDataError, "Not a form body" };
  }

  return form_body->parts();
}

void Request::Prepare() {
  if (!start_line_.empty()) {
    return;
  }

  if (url_.host().empty()) {
    throw Error{ Error::kSyntaxError, "Host is missing" };
  }

  std::string target = url_.path();
  if (!url_.query().empty()) {
    target += "?";
    target += url_.query();
  }

  start_line_ = method_;
  start_line_ += " ";
  start_line_ += target;
  start_line_ += " HTTP/1.1";

  if (url_.port().empty()) {
    SetHeader(headers::kHost, url_.host());
  } else {
    SetHeader(headers::kHost, url_.host() + ":" + url_.port());
  }
}

}  // namespace webcc
