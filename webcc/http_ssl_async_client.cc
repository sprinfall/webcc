#include "webcc/http_ssl_async_client.h"

#include "boost/asio/connect.hpp"

#include "webcc/logger.h"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace webcc {

HttpSslAsyncClient::HttpSslAsyncClient(boost::asio::io_context& io_context,
                                       std::size_t buffer_size,
                                       bool ssl_verify)
    : HttpAsyncClientBase(io_context, buffer_size),
      ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context, ssl_context_),
      ssl_verify_(ssl_verify) {
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

void HttpSslAsyncClient::Resolve() {
  DoResolve(kHttpSslPort);
}

void HttpSslAsyncClient::DoHandshake() {
  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(request_->host()));

  ssl_socket_.async_handshake(ssl::stream_base::client,
                              std::bind(&HttpSslAsyncClient::OnHandshake,
                                        shared_from_this(),
                                        std::placeholders::_1));
}

void HttpSslAsyncClient::OnHandshake(boost::system::error_code ec) {
  if (ec) {
    LOG_ERRO("Handshake error (%s).", ec.message().c_str());
    response_callback_(response_, kHandshakeError, false);
    return;
  }

  DoWrite();
}

void HttpSslAsyncClient::SocketAsyncConnect(const Endpoints& endpoints,
                                            ConnectHandler&& handler) {
  boost::asio::async_connect(ssl_socket_.lowest_layer(), endpoints,
                             std::move(handler));
}

void HttpSslAsyncClient::SocketAsyncWrite(WriteHandler&& handler) {
  boost::asio::async_write(ssl_socket_, request_->ToBuffers(),
                           std::move(handler));
}

void HttpSslAsyncClient::SocketAsyncReadSome(ReadHandler&& handler) {
  ssl_socket_.async_read_some(boost::asio::buffer(buffer_), std::move(handler));
}

void HttpSslAsyncClient::SocketClose(boost::system::error_code* ec) {
  ssl_socket_.lowest_layer().close(*ec);
}

}  // namespace webcc
