#ifndef WEBCC_RESPONSE_H_
#define WEBCC_RESPONSE_H_

#include <memory>
#include <string>

#include "webcc/message.h"

namespace webcc {

class Response : public Message {
public:
  explicit Response(Status status = Status::kOK) : status_(status) {
  }

  ~Response() override = default;

  int status() const {
    return status_;
  }

  void set_status(int status) {
    status_ = status;
  }

  const std::string& reason() const {
    return reason_;
  }

  void set_reason(const std::string& reason) {
    reason_ = reason;
  }

  void Prepare() override;

private:
  int status_;  // Status code
  std::string reason_;  // Reason phrase
};

using ResponsePtr = std::shared_ptr<Response>;

}  // namespace webcc

#endif  // WEBCC_RESPONSE_H_
