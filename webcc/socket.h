#ifndef WEBCC_SOCKET_H_
#define WEBCC_SOCKET_H_

#include <vector>

#include "boost/asio/ip/tcp.hpp"

#include "webcc/config.h"
#include "webcc/request.h"

#if WEBCC_ENABLE_SSL
#include "boost/asio/ssl.hpp"
#endif  // WEBCC_ENABLE_SSL

namespace webcc {

// -----------------------------------------------------------------------------

class SocketBase {
public:
  using Endpoints = boost::asio::ip::tcp::resolver::results_type;

  using ConnectHandler = std::function<void(boost::system::error_code,
                                            boost::asio::ip::tcp::endpoint)>;

  using WriteHandler =
      std::function<void(boost::system::error_code, std::size_t)>;

  using ReadHandler =
      std::function<void(boost::system::error_code, std::size_t)>;

  virtual ~SocketBase() = default;

  virtual void AsyncConnect(const std::string& host, const Endpoints& endpoints,
                            ConnectHandler&& handler) = 0;

  virtual void AsyncWrite(const Payload& payload, WriteHandler&& handler) = 0;

  virtual void AsyncReadSome(ReadHandler&& handler,
                             std::vector<char>* buffer) = 0;

  virtual bool Shutdown() = 0;

  virtual bool Close() = 0;
};

// -----------------------------------------------------------------------------

class Socket : public SocketBase {
public:
  explicit Socket(boost::asio::io_context& io_context);

  void AsyncConnect(const std::string& host, const Endpoints& endpoints,
                    ConnectHandler&& handler) override;

  void AsyncWrite(const Payload& payload, WriteHandler&& handler) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  bool Shutdown() override;

  bool Close() override;

private:
  boost::asio::ip::tcp::socket socket_;
};

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

class SslSocket : public SocketBase {
public:
  SslSocket(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context,
            bool ssl_verify = true);

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

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

  // Verify the certificate of the peer (remote server) or not.
  bool ssl_verify_ = true;
};

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc

#endif  // WEBCC_SOCKET_H_
