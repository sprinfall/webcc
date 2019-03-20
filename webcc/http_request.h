#ifndef WEBCC_HTTP_REQUEST_H_
#define WEBCC_HTTP_REQUEST_H_

#include <memory>
#include <string>
#include <vector>

#include "webcc/http_message.h"
#include "webcc/url.h"

namespace webcc {

class HttpRequest;

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

class HttpRequest : public HttpMessage {
public:
  HttpRequest() = default;

  HttpRequest(const std::string& method, const std::string& url)
      : method_(method), url_(url) {
  }

  ~HttpRequest() override = default;

  void set_method(const std::string& method) {
    method_ = method;
  }

  void set_url(const std::string& url) {
    url_.Init(url);
  }

  // Add URL query parameter.
  void AddParameter(const std::string& key, const std::string& value) {
    url_.AddParameter(key, value);
  }

  const std::string& method() const {
    return method_;
  }

  const Url& url() const {
    return url_;
  }

  const std::string& host() const {
    return url_.host();
  }

  const std::string& port() const {
    return url_.port();
  }

  std::string port(const std::string& default_port) const {
    return port().empty() ? default_port : port();
  }

  // Prepare payload.
  // Compose start line, set Host header, etc.
  bool Prepare() final;

private:
  std::string method_;
  Url url_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_H_
