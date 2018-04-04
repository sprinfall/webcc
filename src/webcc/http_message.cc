#include "webcc/http_message.h"

namespace webcc {

void HttpMessage::SetHeader(const std::string& name, const std::string& value) {
  for (HttpHeader& h : headers_) {
    if (h.name == name) {
      h.value = value;
      return;
    }
  }

  headers_.push_back({ name, value });
}

}  // namespace webcc
