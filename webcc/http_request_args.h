#ifndef WEBCC_HTTP_REQUEST_ARGS_H_
#define WEBCC_HTTP_REQUEST_ARGS_H_

#include <string>
#include <utility>
#include <vector>

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
    LOG_VERB("HttpRequestArgs()");
  }

  HttpRequestArgs(const HttpRequestArgs&) = default;
  HttpRequestArgs& operator=(const HttpRequestArgs&) = default;

#if WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  HttpRequestArgs(HttpRequestArgs&&) = default;
  HttpRequestArgs& operator=(HttpRequestArgs&&) = default;

#else

  HttpRequestArgs(HttpRequestArgs&& rhs)
      : method_(std::move(rhs.method_)),
        url_(std::move(rhs.url_)),
        parameters_(std::move(rhs.parameters_)),
        data_(std::move(rhs.data_)),
        json_(rhs.json_),
        headers_(std::move(rhs.headers_)),
        buffer_size_(rhs.buffer_size_) {
    LOG_VERB("HttpRequestArgs(&&)");
  }

  HttpRequestArgs& operator=(HttpRequestArgs&& rhs) {
    if (&rhs != this) {
      method_ = std::move(rhs.method_);
      url_ = std::move(rhs.url_);
      parameters_ = std::move(rhs.parameters_);
      data_ = std::move(rhs.data_);
      json_ = rhs.json_;
      headers_ = std::move(rhs.headers_);
      buffer_size_ = buffer_size_;
    }
    LOG_VERB("HttpRequestArgs& operator=(&&)");
    return *this;
  }

#endif  // WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  HttpRequestArgs&& method(const std::string& method) {
    method_ = method;
    return std::move(*this);
  }

  HttpRequestArgs&& url(const std::string& url) {
    url_ = url;
    return std::move(*this);
  }

  HttpRequestArgs&& url(std::string&& url) {
    url_ = std::move(url);
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

  HttpRequestArgs&& buffer_size(std::size_t buffer_size) {
    buffer_size_ = buffer_size;
    return std::move(*this);
  }

private:
  friend class HttpClientSession;

  std::string method_;

  std::string url_;

  std::vector<std::string> parameters_;

  // Data to send in the body of the request.
  std::string data_;

  // Is the data to send a JSON string?
  bool json_;

  std::vector<std::string> headers_;

  // Size of the buffer to read response.
  // Leave it to 0 for using default value.
  std::size_t buffer_size_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_ARGS_H_
