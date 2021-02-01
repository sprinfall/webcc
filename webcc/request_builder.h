#ifndef WEBCC_REQUEST_BUILDER_H_
#define WEBCC_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "webcc/request.h"
#include "webcc/url.h"

// -----------------------------------------------------------------------------
// Handy macros for creating a RequestBuilder.

#define WEBCC_GET(url) webcc::RequestBuilder{}.Get(url, false)
#define WEBCC_GET_ENC(url) webcc::RequestBuilder{}.Get(url, true)
#define WEBCC_HEAD(url) webcc::RequestBuilder{}.Head(url, false)
#define WEBCC_HEAD_ENC(url) webcc::RequestBuilder{}.Head(url, true)
#define WEBCC_POST(url) webcc::RequestBuilder{}.Post(url, false)
#define WEBCC_POST_ENC(url) webcc::RequestBuilder{}.Post(url, true)
#define WEBCC_PUT(url) webcc::RequestBuilder{}.Put(url, false)
#define WEBCC_PUT_ENC(url) webcc::RequestBuilder{}.Put(url, true)
#define WEBCC_DELETE(url) webcc::RequestBuilder{}.Delete(url, false)
#define WEBCC_DELETE_ENC(url) webcc::RequestBuilder{}.Delete(url, true)
#define WEBCC_PATCH(url) webcc::RequestBuilder{}.Patch(url, false)
#define WEBCC_PATCH_ENC(url) webcc::RequestBuilder{}.Patch(url, true)

// -----------------------------------------------------------------------------

namespace webcc {

class RequestBuilder {
public:
  RequestBuilder() = default;

  RequestBuilder(const RequestBuilder&) = delete;
  RequestBuilder& operator=(const RequestBuilder&) = delete;

  // Build
  RequestPtr operator()();

  RequestBuilder& Method(const std::string& method) {
    method_ = method;
    return *this;
  }

  RequestBuilder& Get(const std::string& url, bool encode = false) {
    return Method(methods::kGet).Url(url, encode);
  }

  RequestBuilder& Head(const std::string& url, bool encode = false) {
    return Method(methods::kHead).Url(url, encode);
  }

  RequestBuilder& Post(const std::string& url, bool encode = false) {
    return Method(methods::kPost).Url(url, encode);
  }

  RequestBuilder& Put(const std::string& url, bool encode = false) {
    return Method(methods::kPut).Url(url, encode);
  }

  RequestBuilder& Delete(const std::string& url, bool encode = false) {
    return Method(methods::kDelete).Url(url, encode);
  }

  RequestBuilder& Patch(const std::string& url, bool encode = false) {
    return Method(methods::kPatch).Url(url, encode);
  }

  RequestBuilder& Url(const std::string& url, bool encode = false) {
    url_ = webcc::Url{ url, encode };
    return *this;
  }

  RequestBuilder& Port(const std::string& port) {
    url_.set_port(port);
    return *this;
  }

  RequestBuilder& Port(std::uint16_t port) {
    url_.set_port(std::to_string(port));
    return *this;
  }

  // Append a piece to the path.
  RequestBuilder& Path(const std::string& path, bool encode = false) {
    url_.AppendPath(path, encode);
    return *this;
  }

  // Append a parameter to the query.
  RequestBuilder& Query(const std::string& key, const std::string& value,
                        bool encode = false) {
    url_.AppendQuery(key, value, encode);
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

  // Set (comma separated) content types to accept.
  // E.g., "application/json", "text/html, application/xhtml+xml".
  RequestBuilder& Accept(const std::string& content_types) {
    return Header(headers::kAccept, content_types);
  }

#ifdef WEBCC_ENABLE_GZIP

  // Accept Gzip compressed response data or not.
  RequestBuilder& AcceptGzip(bool gzip = true);

#endif  // WEBCC_ENABLE_GZIP

  RequestBuilder& Body(const std::string& data) {
    body_.reset(new StringBody{ data, false });
    return *this;
  }

  RequestBuilder& Body(std::string&& data) {
    body_.reset(new StringBody{ std::move(data), false });
    return *this;
  }

  // Use the file content as body.
  // NOTE: Error::kFileError might be thrown.
  RequestBuilder& File(const boost::filesystem::path& path,
                       bool infer_media_type = true,
                       std::size_t chunk_size = 1024);

  // Add a form part.
  RequestBuilder& Form(FormPartPtr part) {
    form_parts_.push_back(part);
    return *this;
  }

  // Add a form part of file.
  RequestBuilder& FormFile(const std::string& name,
                           const boost::filesystem::path& path,
                           const std::string& media_type = "");

  // Add a form part of string data.
  RequestBuilder& FormData(const std::string& name, std::string&& data,
                           const std::string& media_type = "");

  RequestBuilder& Header(const std::string& key, const std::string& value);

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

#ifdef WEBCC_ENABLE_GZIP

  // Compress the body data (only for string body).
  // NOTE:
  // Most servers don't support compressed requests.
  // Even the requests module from Python doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
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

  // The media (or MIME) type of `Content-Type` header.
  // E.g., "application/json".
  std::string media_type_;

  // The charset of `Content-Type` header.
  // E.g., "utf-8".
  std::string charset_;

  // Files to upload for a POST request.
  std::vector<FormPartPtr> form_parts_;

  // Additional headers with the following sequence:
  //   { key1, value1, key2, value2, ... }
  Strings headers_;

  // Persistent connection.
  bool keep_alive_ = true;

#ifdef WEBCC_ENABLE_GZIP
  bool gzip_ = false;
#endif  // WEBCC_ENABLE_GZIP
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_BUILDER_H_
