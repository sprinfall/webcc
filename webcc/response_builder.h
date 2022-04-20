#ifndef WEBCC_RESPONSE_BUILDER_H_
#define WEBCC_RESPONSE_BUILDER_H_

#include "webcc/message_builder.h"
#include "webcc/request.h"
#include "webcc/response.h"

namespace webcc {

class ResponseBuilder : public MessageBuilder<ResponseBuilder> {
public:
  // Note that `request` is necessary only when Gzip is enabled and the client
  // does want to accept Gzip compressed response.
  ResponseBuilder(RequestPtr request = {})
      : MessageBuilder<ResponseBuilder>(this), request_(request) {
  }

  ~ResponseBuilder() override = default;

  // Build
  ResponsePtr operator()();

  ResponseBuilder& Code(int code) {
    code_ = code;
    return *this;
  }

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

private:
  RequestPtr request_;  // Optional

  int code_ = status_codes::kOK;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_BUILDER_H_
