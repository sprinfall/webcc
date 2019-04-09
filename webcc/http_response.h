#ifndef WEBCC_HTTP_RESPONSE_H_
#define WEBCC_HTTP_RESPONSE_H_

#include <memory>
#include <string>

#include "webcc/http_message.h"

namespace webcc {

class HttpResponse;
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

class HttpResponse : public HttpMessage {
public:
  explicit HttpResponse(http::Status status = http::Status::kOK)
      : status_(status) {
  }

  ~HttpResponse() override = default;

  int status() const { return status_; }

  void set_status(int status) { status_ = status; }

  // Set start line according to status code.
  bool Prepare() final;
  
private:
  int status_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_RESPONSE_H_
