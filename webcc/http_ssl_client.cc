#include "webcc/http_ssl_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

namespace ssl = boost::asio::ssl;

namespace webcc {

HttpSslClient::HttpSslClient(std::size_t buffer_size, bool ssl_verify)
    : HttpClientBase(buffer_size),
      ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context_, ssl_context_),
      ssl_verify_(ssl_verify) {
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
}

Error HttpSslClient::Connect(const HttpRequest& request) {
  Error error = DoConnect(request, kHttpSslPort);
  
  if (error != kNoError) {
    return error;
  }

  return Handshake(request.host());
}

// NOTE: Don't check timeout. It doesn't make much sense.
Error HttpSslClient::Handshake(const std::string& host) {
  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host));

  // Use sync API directly since we don't need timeout control.
  boost::system::error_code ec;
  ssl_socket_.handshake(ssl::stream_base::client, ec);

  if (ec) {
    LOG_ERRO("Handshake error (%s).", ec.message().c_str());
    return kHandshakeError;
  }

  return kNoError;
}

void HttpSslClient::SocketConnect(const Endpoints& endpoints,
                                  boost::system::error_code* ec) {
  boost::asio::connect(ssl_socket_.lowest_layer(), endpoints, *ec);
}

void HttpSslClient::SocketWrite(const HttpRequest& request,
                                boost::system::error_code* ec) {
  boost::asio::write(ssl_socket_, request.ToBuffers(), *ec);
}

void HttpSslClient::SocketAsyncReadSome(ReadHandler&& handler) {
  ssl_socket_.async_read_some(boost::asio::buffer(buffer_), std::move(handler));
}

void HttpSslClient::SocketClose(boost::system::error_code* ec) {
  ssl_socket_.lowest_layer().close(*ec);
}

}  // namespace webcc
