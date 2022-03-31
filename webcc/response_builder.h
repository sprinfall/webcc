#ifndef WEBCC_RESPONSE_BUILDER_H_
#define WEBCC_RESPONSE_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/fs.h"
#include "webcc/request.h"
#include "webcc/response.h"

namespace webcc {

class ResponseBuilder {
public:
  ResponseBuilder() = default;

  // NOTE:
  // Currently, `request` is necessary only when Gzip is enabled and the client
  // does want to accept Gzip compressed response.
  explicit ResponseBuilder(RequestPtr request) : request_(request) {
  }

  ResponseBuilder(const ResponseBuilder&) = delete;
  ResponseBuilder& operator=(const ResponseBuilder&) = delete;

  // Build
  ResponsePtr operator()();

  // Some shortcuts for different status codes:

  ResponseBuilder& OK() {
    return Code(status_codes::kOK);
  }

  ResponseBuilder& Created() {
    return Code(status_codes::kCreated);
  }

  ResponseBuilder& BadRequest() {
    return Code(status_codes::kBadRequest);
  }

  ResponseBuilder& NotFound() {
    return Code(status_codes::kNotFound);
  }

  ResponseBuilder& InternalServerError() {
    return Code(status_codes::kInternalServerError);
  }

  ResponseBuilder& NotImplemented() {
    return Code(status_codes::kNotImplemented);
  }

  ResponseBuilder& ServiceUnavailable() {
    return Code(status_codes::kServiceUnavailable);
  }

  ResponseBuilder& Code(int code) {
    code_ = code;
    return *this;
  }

  ResponseBuilder& MediaType(string_view media_type) {
    media_type_ = ToString(media_type);
    return *this;
  }

  ResponseBuilder& Charset(string_view charset) {
    charset_ = ToString(charset);
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

  // Use the file content as body.
  // NOTE: Error::kFileError might be thrown.
  ResponseBuilder& File(const fs::path& path, bool infer_media_type = true,
                        std::size_t chunk_size = 1024);

  ResponseBuilder& Header(string_view key, string_view value);

  // Add the Date header to the response.
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
  int code_ = status_codes::kOK;

  // Response body.
  BodyPtr body_;

  // Media type of the body (e.g., "application/json").
  std::string media_type_;

  // Character set of the body (e.g., "utf-8").
  std::string charset_;

#if WEBCC_ENABLE_GZIP
  // Compress the body data (only for string body).
  bool gzip_ = false;
#endif

  // Additional headers.
  std::vector<std::string> headers_;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_BUILDER_H_
