#include "webcc/http_socket.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

namespace ssl = boost::asio::ssl;

namespace webcc {

// -----------------------------------------------------------------------------

HttpSocket::HttpSocket(boost::asio::io_context& io_context)
    : socket_(io_context) {
}

void HttpSocket::Connect(const std::string& /*host*/,
                         const Endpoints& endpoints,
                         boost::system::error_code* ec) {
  boost::asio::connect(socket_, endpoints, *ec);
}

void HttpSocket::Write(const HttpRequest& request,
                       boost::system::error_code* ec) {
  boost::asio::write(socket_, request.payload(), *ec);
}

void HttpSocket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void HttpSocket::Close(boost::system::error_code* ec) {
  socket_.close(*ec);
}

// -----------------------------------------------------------------------------

HttpSslSocket::HttpSslSocket(boost::asio::io_context& io_context,
                             bool ssl_verify)
    : ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context, ssl_context_),
      ssl_verify_(ssl_verify) {
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

void HttpSslSocket::Connect(const std::string& host,
                            const Endpoints& endpoints,
                            boost::system::error_code* ec) {
  boost::asio::connect(ssl_socket_.lowest_layer(), endpoints, *ec);

  if (*ec) {
    return;
  }

  Handshake(host, ec);
}

void HttpSslSocket::Write(const HttpRequest& request,
                          boost::system::error_code* ec) {
  boost::asio::write(ssl_socket_, request.payload(), *ec);
}

void HttpSslSocket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  ssl_socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void HttpSslSocket::Close(boost::system::error_code* ec) {
  ssl_socket_.lowest_layer().close(*ec);
}

void HttpSslSocket::Handshake(const std::string& host,
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

}  // namespace webcc
