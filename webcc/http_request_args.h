#ifndef WEBCC_HTTP_REQUEST_ARGS_H_
#define WEBCC_HTTP_REQUEST_ARGS_H_

#include <string>
#include <utility>
#include <vector>

#include "boost/optional.hpp"

#include "webcc/globals.h"
#include "webcc/logger.h"

namespace webcc {

class HttpClientSession;

// Args maker for HttpClientSession.
// Use method chaining to simulate Named Parameters.
class HttpRequestArgs {
public:
  explicit HttpRequestArgs(const std::string& method = "")
      : method_(method), json_(false), buffer_size_(0) {
  }

  HttpRequestArgs(const HttpRequestArgs&) = default;
  HttpRequestArgs& operator=(const HttpRequestArgs&) = default;

  HttpRequestArgs(HttpRequestArgs&&) = default;
  HttpRequestArgs& operator=(HttpRequestArgs&&) = default;

  HttpRequestArgs&& method(const std::string& method) {
    method_ = method;
    return std::move(*this);
  }

  HttpRequestArgs&& url(const std::string& url) {
    url_ = url;
    return std::move(*this);
  }

  HttpRequestArgs&& parameters(const std::vector<std::string>& parameters) {
    parameters_ = parameters;
    return std::move(*this);
  }

  HttpRequestArgs&& parameters(std::vector<std::string>&& parameters) {
    parameters_ = std::move(parameters);
    return std::move(*this);
  }

  HttpRequestArgs&& data(const std::string& data) {
    data_ = data;
    return std::move(*this);
  }

  HttpRequestArgs&& data(std::string&& data) {
    data_ = std::move(data);
    return std::move(*this);
  }

  HttpRequestArgs&& json(bool json = true) {
    json_ = json;
    return std::move(*this);
  }

  HttpRequestArgs&& headers(const std::vector<std::string>& headers) {
    headers_ = headers;
    return std::move(*this);
  }

  HttpRequestArgs&& headers(std::vector<std::string>&& headers) {
    headers_ = std::move(headers);
    return std::move(*this);
  }

  HttpRequestArgs&& ssl_verify(bool ssl_verify = true) {
    ssl_verify_.emplace(ssl_verify);
    return std::move(*this);
  }

  HttpRequestArgs&& buffer_size(std::size_t buffer_size) {
    buffer_size_ = buffer_size;
    return std::move(*this);
  }

private:
  friend class HttpClientSession;

  std::string method_;

  std::string url_;

  // URL query parameters.
  std::vector<std::string> parameters_;

  // Data to send in the body of the request.
  std::string data_;

  // Is the data to send a JSON string?
  bool json_;

  // Additional request headers.
  std::vector<std::string> headers_;

  // Verify the certificate of the peer or not.
  boost::optional<bool> ssl_verify_;

  // Size of the buffer to read response.
  // Leave it to 0 for using default value.
  std::size_t buffer_size_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_ARGS_H_
