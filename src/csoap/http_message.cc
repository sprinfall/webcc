#include "csoap/http_message.h"

namespace csoap {

void HttpMessage::SetHeader(const std::string& name, const std::string& value) {
  for (HttpHeader& h : headers_) {
    if (h.name == name) {  // TODO: Ignore case?
      h.value = value;
      return;
    }
  }

  headers_.push_back({ name, value });
}

}  // namespace csoap
