#ifndef WEBCC_RESPONSE_H_
#define WEBCC_RESPONSE_H_

#include <memory>
#include <string>

#include "webcc/message.h"

namespace webcc {

class Response;
using ResponsePtr = std::shared_ptr<Response>;

class Response : public Message {
public:
  explicit Response(Status status = Status::kOK)
      : status_(status) {
  }

  ~Response() override = default;

  int status() const {
    return status_;
  }

  void set_status(int status) {
    status_ = status;
  }

  // Set start line according to status code.
  void Prepare() override;

private:
  int status_;
};

}  // namespace webcc

#endif  // WEBCC_RESPONSE_H_
