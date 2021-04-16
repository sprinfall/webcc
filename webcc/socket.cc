#include "webcc/socket.h"

#if WEBCC_ENABLE_SSL
#if (defined(_WIN32) || defined(_WIN64))

#include <cryptuiapi.h>
#include <wincrypt.h>
#include <windows.h>

#include "openssl/x509.h"

#endif  // defined(_WIN32) || defined(_WIN64)
#endif  // WEBCC_ENABLE_SSL

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

using boost::asio::ip::tcp;
using namespace std::placeholders;

namespace webcc {

// -----------------------------------------------------------------------------

Socket::Socket(boost::asio::io_context& io_context) : socket_(io_context) {
}

void Socket::AsyncConnect(const std::string& host, const Endpoints& endpoints,
                          ConnectHandler&& handler) {
  boost::asio::async_connect(socket_, endpoints, std::move(handler));
}

void Socket::AsyncWrite(const Payload& payload, WriteHandler&& handler) {
  boost::asio::async_write(socket_, payload, std::move(handler));
}

void Socket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

bool Socket::Shutdown() {
  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
    return false;
  }

  return true;
}

bool Socket::Close() {
  boost::system::error_code ec;
  socket_.close(ec);

  if (ec) {
    LOG_WARN("Socket close error (%s)", ec.message().c_str());
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

namespace ssl = boost::asio::ssl;

SslSocket::SslSocket(boost::asio::io_context& io_context,
                     ssl::context& ssl_context, bool ssl_verify)
    : ssl_socket_(io_context, ssl_context),
      ssl_verify_(ssl_verify) {
}

void SslSocket::AsyncConnect(const std::string& host,
                             const Endpoints& endpoints,
                             ConnectHandler&& handler) {
  connect_handler_ = std::move(handler);


  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  // ssl::host_name_verification has been added since Boost 1.73 to replace
  // ssl::rfc2818_verification.
#if BOOST_VERSION < 107300
  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host));
#else
  ssl_socket_.set_verify_callback(ssl::host_name_verification(host));
#endif  // BOOST_VERSION < 107300

  boost::asio::async_connect(ssl_socket_.lowest_layer(), endpoints,
                             std::bind(&SslSocket::OnConnect, this, _1, _2));
}

void SslSocket::AsyncWrite(const Payload& payload, WriteHandler&& handler) {
  boost::asio::async_write(ssl_socket_, payload, std::move(handler));
}

void SslSocket::AsyncReadSome(ReadHandler&& handler,
                              std::vector<char>* buffer) {
  ssl_socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

bool SslSocket::Shutdown() {
  boost::system::error_code ec;
  ssl_socket_.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
    return false;
  }

  return true;
}

bool SslSocket::Close() {
  boost::system::error_code ec;
  ssl_socket_.lowest_layer().close(ec);

  if (ec) {
    LOG_WARN("Socket close error (%s)", ec.message().c_str());
    return false;
  }

  return true;
}

void SslSocket::OnConnect(boost::system::error_code ec,
                          tcp::endpoint endpoint) {
  if (ec) {
    connect_handler_(ec, std::move(endpoint));
    return;
  }

  // Backup endpoint
  endpoint_ = std::move(endpoint);

  ssl_socket_.async_handshake(ssl::stream_base::client,
                              [this](boost::system::error_code ec) {
    if (ec) {
      LOG_ERRO("Handshake error (%s)", ec.message().c_str());
    }

    connect_handler_(ec, std::move(endpoint_));
  });
}

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc
