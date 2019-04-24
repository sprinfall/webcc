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
  virtual ~SocketBase() = default;

  using Endpoints = boost::asio::ip::tcp::resolver::results_type;

  using ReadHandler =
      std::function<void(boost::system::error_code, std::size_t)>;

  // TODO: Remove |host|
  virtual void Connect(const std::string& host,
                       const Endpoints& endpoints,
                       boost::system::error_code* ec) = 0;

  virtual void Write(const Request& request,
                     boost::system::error_code* ec) = 0;

  virtual void AsyncReadSome(ReadHandler&& handler,
                             std::vector<char>* buffer) = 0;

  virtual void Close(boost::system::error_code* ec) = 0;
};

// -----------------------------------------------------------------------------

class Socket : public SocketBase {
public:
  explicit Socket(boost::asio::io_context& io_context);

  void Connect(const std::string& host,
               const Endpoints& endpoints,
               boost::system::error_code* ec) override;

  void Write(const Request& request,
             boost::system::error_code* ec) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  void Close(boost::system::error_code* ec) override;

private:
  boost::asio::ip::tcp::socket socket_;
};

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

class SslSocket : public SocketBase {
public:
  explicit SslSocket(boost::asio::io_context& io_context,
                     bool ssl_verify = true);

  void Connect(const std::string& host,
               const Endpoints& endpoints,
               boost::system::error_code* ec) override;

  void Write(const Request& request,
             boost::system::error_code* ec) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  void Close(boost::system::error_code* ec) override;

private:
  void Handshake(const std::string& host, boost::system::error_code* ec);

  boost::asio::ssl::context ssl_context_;

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

  // Verify the certificate of the peer (remote server) or not.
  bool ssl_verify_;
};

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc

#endif  // WEBCC_SOCKET_H_
