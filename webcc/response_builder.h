#ifndef WEBCC_RESPONSE_BUILDER_H_
#define WEBCC_RESPONSE_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/request.h"
#include "webcc/response.h"

namespace webcc {

class ResponseBuilder {
public:
  ResponseBuilder() = default;

  // NOTE:
  // Currently, |request| is necessary only when Gzip is enabled and the client
  // does want to accept Gzip compressed response.
  explicit ResponseBuilder(RequestPtr request) : request_(request) {
  }

  ResponseBuilder(const ResponseBuilder&) = delete;
  ResponseBuilder& operator=(const ResponseBuilder&) = delete;

  // Build the response.
  ResponsePtr operator()();

  // NOTE:
  // The naming convention doesn't follow Google C++ Style for
  // consistency and simplicity.

  // Some shortcuts for different status codes:

  ResponseBuilder& OK() {
    return Code(Status::kOK);
  }

  ResponseBuilder& Created() {
    return Code(Status::kCreated);
  }

  ResponseBuilder& BadRequest() {
    return Code(Status::kBadRequest);
  }

  ResponseBuilder& NotFound() {
    return Code(Status::kNotFound);
  }

  ResponseBuilder& InternalServerError() {
    return Code(Status::kInternalServerError);
  }

  ResponseBuilder& NotImplemented() {
    return Code(Status::kNotImplemented);
  }

  ResponseBuilder& ServiceUnavailable() {
    return Code(Status::kServiceUnavailable);
  }

  ResponseBuilder& Code(Status code) {
    code_ = code;
    return *this;
  }

  ResponseBuilder& MediaType(const std::string& media_type) {
    media_type_ = media_type;
    return *this;
  }

  ResponseBuilder& Charset(const std::string& charset) {
    charset_ = charset;
    return *this;
  }

  // Set Media Type to "application/json".
  ResponseBuilder& Json() {
    media_type_ = media_types::kApplicationJson;
    return *this;
  }

  // Set Charset to "utf-8".
  ResponseBuilder& Utf8() {
    charset_ = charsets::kUtf8;
    return *this;
  }

  ResponseBuilder& Body(const std::string& data) {
    body_.reset(new StringBody{ data, false });
    return *this;
  }

  ResponseBuilder& Body(std::string&& data) {
    body_.reset(new StringBody{ std::move(data), false });
    return *this;
  }

  ResponseBuilder& Header(const std::string& key, const std::string& value) {
    headers_.push_back(key);
    headers_.push_back(value);
    return *this;
  }

  // Add the `Date` header to the response.
  ResponseBuilder& Date();

#if WEBCC_ENABLE_GZIP
  ResponseBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *this;
  }
#endif  // WEBCC_ENABLE_GZIP

private:
  RequestPtr request_;

  // Status code.
  Status code_ = Status::kOK;

  // Response body.
  BodyPtr body_;

  // Media type of the body (e.g., "application/json").
  std::string media_type_;

  // Character set of the body (e.g., "utf-8").
  std::string charset_;

#if WEBCC_ENABLE_GZIP
  // Compress the body data (only for string body).
  bool gzip_ = false;
#endif  // WEBCC_ENABLE_GZIP

  // Additional headers.
  std::vector<std::string> headers_;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_BUILDER_H_
