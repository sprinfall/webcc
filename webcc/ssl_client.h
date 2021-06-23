#ifndef WEBCC_SSL_CLIENT_H_
#define WEBCC_SSL_CLIENT_H_

#include "webcc/client.h"

#include "boost/asio/ssl/context.hpp"

#if !WEBCC_ENABLE_SSL
#error SSL must be enabled!
#endif

namespace webcc {

class SslClient final : public Client {
public:
  SslClient(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context);

  ~SslClient() = default;

protected:
  void AsyncConnect() override;

private:
  boost::asio::ssl::context& ssl_context_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CLIENT_H_
