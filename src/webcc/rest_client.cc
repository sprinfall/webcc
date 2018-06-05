#include "webcc/rest_client.h"

#include "webcc/http_client.h"
#include "webcc/http_request.h"

namespace webcc {

bool RestClient::Request(const std::string& method,
                         const std::string& url,
                         const std::string& content) {
  response_.reset();
  error_ = kNoError;
  timeout_occurred_ = false;

  HttpRequest request;

  request.set_method(method);
  request.set_url(url);
  request.SetHost(host_, port_);

  if (!content.empty()) {
    request.SetContent(content);
  }

  request.Build();

  HttpClient http_client;
  http_client.set_timeout_seconds(timeout_seconds_);

  if (!http_client.Request(request)) {
    error_ = http_client.error();
    timeout_occurred_ = http_client.timeout_occurred();
    return false;
  }

  response_ = http_client.response();
  return true;
}

}  // namespace webcc
