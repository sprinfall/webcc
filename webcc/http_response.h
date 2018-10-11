#ifndef WEBCC_HTTP_RESPONSE_H_
#define WEBCC_HTTP_RESPONSE_H_

#include <memory>
#include <string>

#include "webcc/http_message.h"

namespace webcc {

class HttpResponse : public HttpMessage {
 public:
  HttpResponse() : status_(HttpStatus::kOK) {}

  ~HttpResponse() override = default;

  int status() const { return status_; }

  void set_status(int status) { status_ = status; }

  // Set start line according to status code.
  void Make() override;

  // Get a fault response when HTTP status is not OK.
  // TODO: Avoid copy.
  static HttpResponse Fault(HttpStatus::Enum status);

 private:
  int status_;
};

typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_RESPONSE_H_
