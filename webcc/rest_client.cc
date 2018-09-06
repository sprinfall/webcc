#include "webcc/rest_client.h"

#include "webcc/http_request.h"

namespace webcc {

RestClient::RestClient(const std::string& host, const std::string& port)
    : host_(host), port_(port) {
  if (port_.empty()) {
    std::size_t i = host_.find_last_of(':');
    if (i != std::string::npos) {
      port_ = host_.substr(i + 1);
      host_ = host_.substr(0, i);
    }
  }
}

bool RestClient::Request(const std::string& method, const std::string& url,
                         std::string&& content) {
  HttpRequest http_request;

  http_request.set_method(method);
  http_request.set_url(url);
  http_request.SetHost(host_, port_);

  if (!content.empty()) {
    http_request.SetContent(std::move(content), true);
    http_request.SetContentType(kAppJsonUtf8);
  }

  http_request.UpdateStartLine();

  if (!http_client_.Request(http_request)) {
    return false;
  }

  return true;
}

}  // namespace webcc
