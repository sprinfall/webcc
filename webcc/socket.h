#ifndef WEBCC_SOCKET_H_
#define WEBCC_SOCKET_H_

#include <vector>

#include "asio/ip/tcp.hpp"

#include "webcc/config.h"
#include "webcc/request.h"

#if WEBCC_ENABLE_SSL
#include "asio/ssl.hpp"
#endif  // WEBCC_ENABLE_SSL

namespace webcc {

// -----------------------------------------------------------------------------

class SocketBase {
public:
  virtual ~SocketBase() = default;

  using Endpoints = asio::ip::tcp::resolver::results_type;

  using ReadHandler =
      std::function<void(std::error_code, std::size_t)>;

  // TODO: Remove |host|
  virtual bool Connect(const std::string& host, const Endpoints& endpoints) = 0;

  virtual bool Write(const Payload& payload, std::error_code* ec) = 0;

  virtual bool ReadSome(std::vector<char>* buffer, std::size_t* size,
                        std::error_code* ec) = 0;

  virtual void AsyncReadSome(ReadHandler&& handler,
                             std::vector<char>* buffer) = 0;

  virtual bool Close() = 0;
};

// -----------------------------------------------------------------------------

class Socket : public SocketBase {
public:
  explicit Socket(asio::io_context& io_context);

  bool Connect(const std::string& host, const Endpoints& endpoints) override;

  bool Write(const Payload& payload, std::error_code* ec) override;

  bool ReadSome(std::vector<char>* buffer, std::size_t* size,
                std::error_code* ec) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  bool Close() override;

private:
  asio::ip::tcp::socket socket_;
};

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

class SslSocket : public SocketBase {
public:
  explicit SslSocket(asio::io_context& io_context,
                     bool ssl_verify = true);

  bool Connect(const std::string& host, const Endpoints& endpoints) override;

  bool Write(const Payload& payload, std::error_code* ec) override;

  bool ReadSome(std::vector<char>* buffer, std::size_t* size,
                std::error_code* ec) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  bool Close() override;

private:
  bool Handshake(const std::string& host);

  asio::ssl::context ssl_context_;

  asio::ssl::stream<asio::ip::tcp::socket> ssl_socket_;

  // Verify the certificate of the peer (remote server) or not.
  bool ssl_verify_;
};

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc

#endif  // WEBCC_SOCKET_H_
