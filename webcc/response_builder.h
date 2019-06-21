#ifndef WEBCC_RESPONSE_BUILDER_H_
#define WEBCC_RESPONSE_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/response.h"

namespace webcc {

class ResponseBuilder {
public:
  ResponseBuilder() = default;

  ResponseBuilder(const ResponseBuilder&) = delete;
  ResponseBuilder& operator=(const ResponseBuilder&) = delete;

  // Build the response.
  ResponsePtr operator()();

  // NOTE:
  // The naming convention doesn't follow Google C++ Style for
  // consistency and simplicity.

  // Some shortcuts for different status codes:
  ResponseBuilder& OK() { return Code(Status::kOK); }
  ResponseBuilder& Created() { return Code(Status::kCreated); }
  ResponseBuilder& BadRequest() { return Code(Status::kBadRequest); }
  ResponseBuilder& NotFound() { return Code(Status::kNotFound); }
  ResponseBuilder& NotImplemented() { return Code(Status::kNotImplemented); }

  ResponseBuilder& Code(Status code) {
    code_ = code;
    return *this;
  }

  ResponseBuilder& Data(const std::string& data) {
    data_ = data;
    return *this;
  }

  ResponseBuilder& Data(std::string&& data) {
    data_ = std::move(data);
    return *this;
  }

  ResponseBuilder& Json(bool json = true) {
    json_ = json;
    return *this;
  }

  ResponseBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *this;
  }

  ResponseBuilder& Header(const std::string& key, const std::string& value) {
    headers_.push_back(key);
    headers_.push_back(value);
    return *this;
  }

  // Add the Date header to the response.
  ResponseBuilder& Date();

private:
  void SetContent(ResponsePtr response, std::string&& data);

private:
  // Status code.
  Status code_ = Status::kOK;

  // Data to send in the body of the request.
  std::string data_;

  // Is the data to send a JSON string?
  bool json_ = false;

  // Compress the response content.
  bool gzip_ = false;

  // Additional headers.
  std::vector<std::string> headers_;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_BUILDER_H_
