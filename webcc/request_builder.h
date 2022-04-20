#ifndef WEBCC_REQUEST_BUILDER_H_
#define WEBCC_REQUEST_BUILDER_H_

#include "webcc/base64.h"
#include "webcc/message_builder.h"
#include "webcc/request.h"
#include "webcc/url.h"

// -----------------------------------------------------------------------------
// Handy macros for creating a RequestBuilder.

#define WEBCC_RB webcc::RequestBuilder{}

// clang-format off
#define WEBCC_GET(url)          WEBCC_RB.Get(url, false)
#define WEBCC_GET_ENC(url)      WEBCC_RB.Get(url, true)
#define WEBCC_HEAD(url)         WEBCC_RB.Head(url, false)
#define WEBCC_HEAD_ENC(url)     WEBCC_RB.Head(url, true)
#define WEBCC_POST(url)         WEBCC_RB.Post(url, false)
#define WEBCC_POST_ENC(url)     WEBCC_RB.Post(url, true)
#define WEBCC_PUT(url)          WEBCC_RB.Put(url, false)
#define WEBCC_PUT_ENC(url)      WEBCC_RB.Put(url, true)
#define WEBCC_DELETE(url)       WEBCC_RB.Delete(url, false)
#define WEBCC_DELETE_ENC(url)   WEBCC_RB.Delete(url, true)
#define WEBCC_PATCH(url)        WEBCC_RB.Patch(url, false)
#define WEBCC_PATCH_ENC(url)    WEBCC_RB.Patch(url, true)
// clang-format on

// -----------------------------------------------------------------------------

namespace webcc {

class RequestBuilder : public MessageBuilder<RequestBuilder> {
public:
  RequestBuilder() : MessageBuilder<RequestBuilder>(this) {
  }
 
  ~RequestBuilder() override = default;

  // Build
  RequestPtr operator()();

  RequestBuilder& Method(std::string_view method) {
    method_ = method;
    return *this;
  }

  RequestBuilder& Get(std::string_view url, bool encode = false) {
    return Method(methods::kGet).Url(url, encode);
  }

  RequestBuilder& Head(std::string_view url, bool encode = false) {
    return Method(methods::kHead).Url(url, encode);
  }

  RequestBuilder& Post(std::string_view url, bool encode = false) {
    return Method(methods::kPost).Url(url, encode);
  }

  RequestBuilder& Put(std::string_view url, bool encode = false) {
    return Method(methods::kPut).Url(url, encode);
  }

  RequestBuilder& Delete(std::string_view url, bool encode = false) {
    return Method(methods::kDelete).Url(url, encode);
  }

  RequestBuilder& Patch(std::string_view url, bool encode = false) {
    return Method(methods::kPatch).Url(url, encode);
  }

  RequestBuilder& Url(std::string_view url, bool encode = false) {
    url_ = webcc::Url{ url, encode };
    return *this;
  }

  RequestBuilder& Port(std::string_view port) {
    url_.set_port(port);
    return *this;
  }

  RequestBuilder& Port(std::uint16_t port) {
    url_.set_port(std::to_string(port));
    return *this;
  }

  // Append a piece to the path.
  RequestBuilder& Path(std::string_view path, bool encode = false) {
    url_.AppendPath(path, encode);
    return *this;
  }

  // Append a parameter to the query.
  // NOTE: Don't use std::string_view!
  RequestBuilder& Query(const std::string& key, const std::string& value,
                        bool encode = false) {
    url_.AppendQuery(key, value, encode);
    return *this;
  }

  // Set (comma separated) content types to accept.
  // E.g., "application/json", "text/html, application/xhtml+xml".
  RequestBuilder& Accept(std::string_view content_types) {
    return Header(headers::kAccept, content_types);
  }

#if WEBCC_ENABLE_GZIP
  // Accept Gzip compressed response data or not.
  RequestBuilder& AcceptGzip(bool gzip = true);
#endif

  // Add a form part.
  RequestBuilder& Form(FormPartPtr part) {
    form_parts_.push_back(part);
    return *this;
  }

  // Add a form part of file.
  RequestBuilder& FormFile(std::string_view name, const sfs::path& path,
                           std::string_view media_type = "") {
    assert(!name.empty());
    return Form(FormPart::NewFile(name, path, media_type));
  }

  // Add a form part of string data.
  RequestBuilder& FormData(std::string_view name, std::string&& data,
                           std::string_view media_type = "") {
    assert(!name.empty());
    return Form(FormPart::New(name, std::move(data), media_type));
  }

  // NOTE: Don't use std::string_view!
  RequestBuilder& Auth(const std::string& type,
                       const std::string& credentials) {
    headers_.emplace_back(headers::kAuthorization);
    headers_.emplace_back(type + " " + credentials);
    return *this;
  }

  // NOTE: Don't use std::string_view!
  RequestBuilder& AuthBasic(const std::string& login,
                            const std::string& password) {
    return Auth("Basic", base64::Encode(login + ":" + password));
  }

  // NOTE: Don't use std::string_view!
  RequestBuilder& AuthToken(const std::string& token) {
    return Auth("Token", token);
  }

private:
  std::string method_;

  // Namespace is added to avoid the conflict with Url() method.
  webcc::Url url_;

  // Files to upload for a POST request.
  std::vector<FormPartPtr> form_parts_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_BUILDER_H_
