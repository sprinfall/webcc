#include "webcc/rest_client.h"

#include "webcc/http_client.h"
#include "webcc/http_request.h"

namespace webcc {

RestClient::RestClient(const std::string& host, const std::string& port)
    : host_(host),
      port_(port),
      timeout_seconds_(0),
      timed_out_(false),
      error_(kNoError) {
}

bool RestClient::Request(const std::string& method,
                         const std::string& url,
                         const std::string& content) {
  response_.reset();

  error_ = kNoError;
  timed_out_ = false;

  HttpRequest request;

  request.set_method(method);
  request.set_url(url);
  request.SetHost(host_, port_);

  if (!content.empty()) {
    request.SetContent(content);
  }

  request.Build();

  HttpClient http_client;

  if (timeout_seconds_ > 0) {
    http_client.set_timeout_seconds(timeout_seconds_);
  }

  if (!http_client.Request(request)) {
    error_ = http_client.error();
    timed_out_ = http_client.timed_out();
    return false;
  }

  response_ = http_client.response();
  return true;
}

}  // namespace webcc
