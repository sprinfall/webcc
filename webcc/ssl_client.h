#ifndef WEBCC_SSL_CLIENT_H_
#define WEBCC_SSL_CLIENT_H_

#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "webcc/blocking_client_base.h"

#if !WEBCC_ENABLE_SSL
#error SSL must be enabled!
#endif

namespace webcc {

using SslStream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

class SslClient final : public BlockingClientBase {
public:
  SslClient(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context)
      : BlockingClientBase(io_context, "443"),
        ssl_stream_(io_context, ssl_context) {
  }

  ~SslClient() override = default;

protected:
  SocketType& GetSocket() override {
    return ssl_stream_.lowest_layer();
  }

  void OnConnected() override;

  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  RWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     RWHandler&& handler) override;

  void CloseSocket() override;

private:
  void OnHandshake(boost::system::error_code ec);

  SslStream ssl_stream_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CLIENT_H_
