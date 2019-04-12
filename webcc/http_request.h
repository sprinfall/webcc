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

  void AddQuery(const std::string& key, const std::string& value) {
    url_.AddQuery(key, value);
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

  const std::vector<FormPart>& form_parts() const {
    return form_parts_;
  }

  void set_form_parts_(std::vector<FormPart>&& form_parts) {
    form_parts_ = std::move(form_parts);
  }

  void AddFormPart(FormPart&& form_part) {
    form_parts_.push_back(std::move(form_part));
  }

  // Prepare payload.
  void Prepare() final;

private:
  void CreateStartLine();

private:
  std::string method_;

  Url url_;

  // Files to upload for a POST request.
  std::vector<FormPart> form_parts_;

  std::string boundary_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_H_
