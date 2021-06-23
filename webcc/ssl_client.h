#ifndef WEBCC_SSL_CLIENT_H_
#define WEBCC_SSL_CLIENT_H_

#include "boost/asio/ssl/context.hpp"

#include "webcc/client_base.h"
#include "webcc/ssl_socket.h"

#if !WEBCC_ENABLE_SSL
#error SSL must be enabled!
#endif

namespace webcc {

class SslClient final : public ClientBase {
public:
  SslClient(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context)
      : ClientBase(io_context), ssl_context_(ssl_context) {
  }

  ~SslClient() = default;

protected:
  void CreateSocket() override {
    socket_.reset(new SslSocket{ io_context_, ssl_context_ });
  }

  void Resolve() override {
    AsyncResolve("443");
  }

private:
  boost::asio::ssl::context& ssl_context_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CLIENT_H_
