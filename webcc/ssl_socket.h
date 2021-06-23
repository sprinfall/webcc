#ifndef WEBCC_SSL_SOCKET_H_
#define WEBCC_SSL_SOCKET_H_

#include "webcc/socket_base.h"

#include "boost/asio/ssl.hpp"

#if !WEBCC_ENABLE_SSL
#error SSL must be enabled!
#endif

namespace webcc {

class SslSocket : public SocketBase {
public:
  SslSocket(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context);

  void AsyncConnect(const std::string& host, const Endpoints& endpoints,
                    ConnectHandler&& handler) override;

  void AsyncWrite(const Payload& payload, WriteHandler&& handler) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  bool Shutdown() override;

  bool Close() override;

private:
  void OnConnect(boost::system::error_code ec,
                 boost::asio::ip::tcp::endpoint endpoint);

  ConnectHandler connect_handler_;
  boost::asio::ip::tcp::endpoint endpoint_;

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_SOCKET_H_
