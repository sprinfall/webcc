#ifndef WEBCC_REQUEST_BUILDER_H_
#define WEBCC_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/request.h"
#include "webcc/url.h"

namespace webcc {

class RequestBuilder {
public:
  RequestBuilder() = default;

  RequestBuilder(const RequestBuilder&) = delete;
  RequestBuilder& operator=(const RequestBuilder&) = delete;

  // Build the request.
  RequestPtr operator()();

  // NOTE:
  // The naming convention doesn't follow Google C++ Style for
  // consistency and simplicity.

  RequestBuilder& Method(const std::string& method) {
    method_ = method;
    return *this;
  }

  RequestBuilder& Url(const std::string& url) {
    url_.Init(url);
    return *this;
  }

  RequestBuilder& Get(const std::string& url) {
    return Method(methods::kGet).Url(url);
  }

  RequestBuilder& Head(const std::string& url) {
    return Method(methods::kHead).Url(url);
  }

  RequestBuilder& Post(const std::string& url) {
    return Method(methods::kPost).Url(url);
  }

  RequestBuilder& Put(const std::string& url) {
    return Method(methods::kPut).Url(url);
  }

  RequestBuilder& Delete(const std::string& url) {
    return Method(methods::kDelete).Url(url);
  }

  RequestBuilder& Patch(const std::string& url) {
    return Method(methods::kPatch).Url(url);
  }

  // Add a query parameter.
  RequestBuilder& Query(const std::string& key, const std::string& value) {
    url_.AddQuery(key, value);
    return *this;
  }

  RequestBuilder& MediaType(const std::string& media_type) {
    media_type_ = media_type;
    return *this;
  }

  RequestBuilder& Charset(const std::string& charset) {
    charset_ = charset;
    return *this;
  }

  // Set Media Type to "application/json".
  RequestBuilder& Json() {
    media_type_ = media_types::kApplicationJson;
    return *this;
  }

  // Set Charset to "utf-8".
  RequestBuilder& Utf8() {
    charset_ = charsets::kUtf8;
    return *this;
  }

  RequestBuilder& Body(const std::string& data) {
    body_.reset(new StringBody{ data });
    return *this;
  }

  RequestBuilder& Body(std::string&& data) {
    body_.reset(new StringBody{ std::move(data) });
    return *this;
  }

  // Add a file to upload.
  RequestBuilder& File(const std::string& name, const Path& path,
                       const std::string& media_type = "");

  RequestBuilder& Form(FormPartPtr part) {
    form_parts_.push_back(part);
    return *this;
  }

  RequestBuilder& Form(const std::string& name, std::string&& data,
                       const std::string& media_type = "");

  RequestBuilder& Header(const std::string& key, const std::string& value) {
    headers_.push_back(key);
    headers_.push_back(value);
    return *this;
  }

  RequestBuilder& KeepAlive(bool keep_alive = true) {
    keep_alive_ = keep_alive;
    return *this;
  }

  RequestBuilder& Auth(const std::string& type, const std::string& credentials);

  RequestBuilder& AuthBasic(const std::string& login,
                            const std::string& password);

  RequestBuilder& AuthToken(const std::string& token);

  // Add the `Date` header to the request.
  RequestBuilder& Date();

#if WEBCC_ENABLE_GZIP
  RequestBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *this;
  }
#endif  // WEBCC_ENABLE_GZIP

private:
  std::string method_;

  // Namespace is added to avoid the conflict with `Url()` method.
  webcc::Url url_;

  // Request body.
  BodyPtr body_;

  // Media type of the body (e.g., "application/json").
  std::string media_type_;

  // Character set of the body (e.g., "utf-8").
  std::string charset_;

  // Files to upload for a POST request.
  std::vector<FormPartPtr> form_parts_;

  // Additional headers with the following sequence:
  //   { key1, value1, key2, value2, ... }
  Strings headers_;

  // Persistent connection.
  bool keep_alive_ = true;

#if WEBCC_ENABLE_GZIP
  // Compress the body data (only for string body).
  // NOTE:
  // Most servers don't support compressed requests.
  // Even the requests module from Python doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
  bool gzip_ = false;
#endif  // WEBCC_ENABLE_GZIP
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_BUILDER_H_
