#ifndef WEBCC_BLOCKING_CLIENT_BASE_H_
#define WEBCC_BLOCKING_CLIENT_BASE_H_

#include <condition_variable>
#include <mutex>

#include "webcc/async_client_base.h"

namespace webcc {

class BlockingClientBase : public AsyncClientBase {
public:
  BlockingClientBase(boost::asio::io_context& io_context,
                     std::string_view default_port)
      : AsyncClientBase(io_context, default_port) {
  }

  ~BlockingClientBase() override = default;

  // Send a request to the server.
  // Block until the response has been received.
  Error Send(RequestPtr request, bool stream = false);

protected:
  void RequestEnd() override;

  bool request_finished_ = true;
  std::condition_variable request_cv_;
  std::mutex request_mutex_;
};

using BlockingClientPtr = std::shared_ptr<BlockingClientBase>;

}  // namespace webcc

#endif  // WEBCC_BLOCKING_CLIENT_BASE_H_
