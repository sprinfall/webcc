#ifndef WEBCC_HTTP_REQUEST_H_
#define WEBCC_HTTP_REQUEST_H_

#include <memory>
#include <string>

#include "webcc/http_message.h"

namespace webcc {

class HttpRequest;
class HttpRequestParser;

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

class HttpRequest : public HttpMessage {
 public:
  HttpRequest() = default;

  // The |host| is a descriptive name (e.g., www.google.com) or a numeric IP
  // address (127.0.0.1).
  // The |port| is a numeric number (e.g., 9000). The default value (80 for HTTP
  // or 443 for HTTPS) will be used to connect to server if it's empty.
  HttpRequest(const std::string& method,
              const std::string& url,
              const std::string& host,
              const std::string& port = "");

  ~HttpRequest() override = default;

  const std::string& method() const { return method_; }

  const std::string& url() const { return url_; }

  const std::string& host() const { return host_; }
  const std::string& port() const { return port_; }

  std::string port(const std::string& default_port) const {
    return port_.empty() ? default_port : port_;
  }

  // Prepare payload.
  // Compose start line, set Host header, etc.
  void Prepare() override;

  static HttpRequestPtr Make(const std::string& method,
                             const std::string& url,
                             const std::string& host,
                             const std::string& port = "",
                             bool prepare = true);

 private:
  friend class HttpRequestParser;

  void set_method(const std::string& method) { method_ = method; }
  void set_url(const std::string& url) { url_ = url; }

  // HTTP method.
  std::string method_;

  // Request URL.
  // A complete URL naming the requested resource, or the path component of
  // the URL.
  std::string url_;

  std::string host_;
  std::string port_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_H_
