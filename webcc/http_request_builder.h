#ifndef WEBCC_HTTP_REQUEST_BUILDER_H_
#define WEBCC_HTTP_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "boost/optional.hpp"

#include "webcc/http_request.h"

namespace webcc {

// TODO: Add timeout()

// HTTP request builder.
class HttpRequestBuilder {
public:
  HttpRequestBuilder() = default;

  // Build the request.
  HttpRequestPtr operator()();

  HttpRequestBuilder& Get() { return method(http::kGet); }
  HttpRequestBuilder& Post() { return method(http::kPost); }
  HttpRequestBuilder& Put() { return method(http::kPut); }
  HttpRequestBuilder& Delete() { return method(http::kDelete); }

  HttpRequestBuilder& method(const std::string& method) {
    method_ = method;
    return *this;
  }

  HttpRequestBuilder& url(const std::string& url) {
    url_ = url;
    return *this;
  }

  HttpRequestBuilder& parameter(const std::string& key,
                                const std::string& value) {
    parameters_.push_back(key);
    parameters_.push_back(value);
    return *this;
  }

  HttpRequestBuilder& data(const std::string& data) {
    data_ = data;
    return *this;
  }

  HttpRequestBuilder& data(std::string&& data) {
    data_ = std::move(data);
    return *this;
  }

  HttpRequestBuilder& json(bool json) {
    json_ = json;
    return *this;
  }

  HttpRequestBuilder& gzip(bool gzip) {
    gzip_ = gzip;
    return *this;
  }

  HttpRequestBuilder& header(const std::string& key,
                             const std::string& value) {
    headers_.push_back(key);
    headers_.push_back(value);
    return *this;
  }

  HttpRequestBuilder& ssl_verify(bool ssl_verify = true) {
    ssl_verify_.emplace(ssl_verify);
    return *this;
  }

  HttpRequestBuilder& buffer(std::size_t buffer) {
    buffer_ = buffer;
    return *this;
  }

  HttpRequestBuilder& keep_alive(bool keep_alive) {
    keep_alive_ = keep_alive;
    return *this;
  }

private:
  std::string method_;

  std::string url_;

  // URL query parameters.
  std::vector<std::string> parameters_;

  // Data to send in the body of the request.
  std::string data_;

  // Is the data to send a JSON string?
  bool json_ = false;

  // Compress the request content.
  // NOTE: Most servers don't support compressed requests.
  // Even the requests module from Python doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
  bool gzip_ = false;

  // Additional request headers.
  std::vector<std::string> headers_;

  // Verify the certificate of the peer or not.
  boost::optional<bool> ssl_verify_;

  // Size of the buffer to read response.
  // Leave it to 0 for using default value.
  std::size_t buffer_ = 0;

  // Persistent connection.
  bool keep_alive_ = true;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_BUILDER_H_
