#include "webcc/socket.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/core/ignore_unused.hpp"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

Socket::Socket(boost::asio::io_context& io_context)
    : socket_(io_context) {
}

void Socket::Connect(const std::string& host, const Endpoints& endpoints,
                     boost::system::error_code* ec) {
  boost::ignore_unused(host);

  boost::asio::connect(socket_, endpoints, *ec);
}

void Socket::Write(const Request& request, boost::system::error_code* ec) {
  boost::asio::write(socket_, request.payload(), *ec);
}

void Socket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void Socket::Close(boost::system::error_code* ec) {
  socket_.close(*ec);
}

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

namespace ssl = boost::asio::ssl;

SslSocket::SslSocket(boost::asio::io_context& io_context, bool ssl_verify)
    : ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context, ssl_context_),
      ssl_verify_(ssl_verify) {
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

void SslSocket::Connect(const std::string& host, const Endpoints& endpoints,
                        boost::system::error_code* ec) {
  boost::asio::connect(ssl_socket_.lowest_layer(), endpoints, *ec);

  if (*ec) {
    return;
  }

  Handshake(host, ec);
}

void SslSocket::Write(const Request& request, boost::system::error_code* ec) {
  boost::asio::write(ssl_socket_, request.payload(), *ec);
}

void SslSocket::AsyncReadSome(ReadHandler&& handler,
                              std::vector<char>* buffer) {
  ssl_socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void SslSocket::Close(boost::system::error_code* ec) {
  ssl_socket_.lowest_layer().close(*ec);
}

void SslSocket::Handshake(const std::string& host,
                          boost::system::error_code* ec) {
  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host));

  // Use sync API directly since we don't need timeout control.
  ssl_socket_.handshake(ssl::stream_base::client, *ec);

  if (*ec) {
    LOG_ERRO("Handshake error (%s).", ec->message().c_str());
  }
}

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc
