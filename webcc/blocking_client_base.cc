#include "webcc/blocking_client_base.h"

#include "webcc/logger.h"

namespace webcc {

Error BlockingClientBase::Send(RequestPtr request, bool stream) {
  LOG_INFO("Blocking request begin");

  request_finished_ = false;  // reset

  AsyncClientBase::AsyncSend(request, stream);

  // Wait for the request to be ended.
  // See RequestEnd().
  std::unique_lock<std::mutex> response_lock{ request_mutex_ };
  request_cv_.wait(response_lock, [=] { return request_finished_; });

  LOG_INFO("Blocking request end");

  return error_;
}

void BlockingClientBase::RequestEnd() {
  request_mutex_.lock();

  if (!request_finished_) {
    request_finished_ = true;

    request_mutex_.unlock();
    request_cv_.notify_one();
  } else {
    request_mutex_.unlock();
  }
}

}  // namespace webcc
