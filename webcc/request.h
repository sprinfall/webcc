#ifndef WEBCC_REQUEST_H_
#define WEBCC_REQUEST_H_

#include <memory>
#include <string>
#include <vector>

#include "webcc/message.h"
#include "webcc/url.h"

namespace webcc {

class Request;
using RequestPtr = std::shared_ptr<Request>;

class Request : public Message {
public:
  Request() = default;

  Request(const std::string& method, const std::string& url)
      : method_(method), url_(url) {
  }

  ~Request() override = default;

  const std::string& method() const {
    return method_;
  }

  void set_method(const std::string& method) {
    method_ = method;
  }

  const Url& url() const {
    return url_;
  }

  void set_url(const std::string& url) {
    url_.Init(url);
  }

  void AddQuery(const std::string& key, const std::string& value) {
    url_.AddQuery(key, value);
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

  const std::vector<FormPartPtr>& form_parts() const {
    return form_parts_;
  }

  void set_form_parts(std::vector<FormPartPtr>&& form_parts) {
    form_parts_ = std::move(form_parts);
  }

  void AddFormPart(FormPartPtr form_part) {
    form_parts_.push_back(form_part);
  }

  // Prepare payload.
  void Prepare() final;

private:
  void CreateStartLine();

private:
  std::string method_;

  Url url_;

  std::vector<FormPartPtr> form_parts_;

  std::string boundary_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_H_
