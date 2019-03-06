#ifndef WEBCC_HTTP_REQUEST_H_
#define WEBCC_HTTP_REQUEST_H_

#include <memory>
#include <string>
#include <vector>

#include "webcc/http_message.h"
#include "webcc/url.h"

namespace webcc {

class HttpRequest;
class HttpRequestParser;

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

class HttpRequest : public HttpMessage {
public:
  HttpRequest() = default;

  // TODO: Move parameters
  HttpRequest(const std::string& method,
              const std::string& url,
              const std::vector<std::string>& parameters = {});

  ~HttpRequest() override = default;

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

  // Shortcut to set `Accept` header.
  void Accept(const std::string& media_type) {
    SetHeader(http::headers::kAccept, media_type);
  }

  // Shortcut to set `Accept` header.
  void AcceptAppJson() {
    SetHeader(http::headers::kAccept, http::media_types::kApplicationJson);
  }

  // Prepare payload.
  // Compose start line, set Host header, etc.
  bool Prepare() override;

  // TODO: Re-place
  static HttpRequestPtr New(const std::string& method,
                            const std::string& url,
                            const std::vector<std::string>& parameters = {},
                            bool prepare = true);

private:
  friend class HttpRequestParser;

  void set_method(const std::string& method) {
    method_ = method;
  }
  void set_url(const std::string& url) {
    url_.Init(url);
  }

  std::string method_;
  Url url_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_H_
