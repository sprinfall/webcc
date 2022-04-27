#include "webcc/ssl_client.h"

#include "boost/asio/write.hpp"
#include "boost/asio/ssl.hpp"

#include "webcc/logger.h"

using namespace std::placeholders;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace webcc {

void SslClient::Close() {
  boost::system::error_code ec;
  GetSocket().cancel(ec);

  // Shutdown SSL
  ssl_stream_.shutdown(ec);
  if (ec) {
    // See: https://stackoverflow.com/a/25703699
    LOG_WARN("SSL shutdown error (%s)", ec.message().c_str());
    ec.clear();
  }

  BlockingClientBase::Close();
}

void SslClient::AsyncWrite(
    const std::vector<boost::asio::const_buffer>& buffers,
    RWHandler&& handler) {
  boost::asio::async_write(ssl_stream_, buffers, std::move(handler));
}

void SslClient::AsyncReadSome(boost::asio::mutable_buffer buffer,
                              RWHandler&& handler) {
  ssl_stream_.async_read_some(buffer, std::move(handler));
}

void SslClient::OnConnected() {
  const std::string& host = request_->host();

  // Modes `ssl::verify_fail_if_no_peer_cert` and `ssl::verify_client_once` are
  // for server only while mode `ssl::verify_none` is not secure.
  // See: https://stackoverflow.com/a/12621528
  ssl_stream_.set_verify_mode(ssl::verify_peer);

  if (ssl_verify_ == SslVerify::kHostName) {
    // Set SNI (server name indication) host name.
    // Many hosts need this to handshake successfully (e.g., google.com).
    // Inspired by Boost.Beast.
    if (!SSL_set_tlsext_host_name(ssl_stream_.native_handle(), host.c_str())) {
      LOG_ERRO("Failed to set SNI host name for SSL");
    }

    ssl_stream_.set_verify_callback(ssl::host_name_verification{ host });

  } else {
    // Don't set a verify callback.
  }

  auto self = std::dynamic_pointer_cast<SslClient>(shared_from_this());

  ssl_stream_.async_handshake(ssl::stream_base::client,
                              std::bind(&SslClient::OnHandshake, self, _1));
}

void SslClient::OnHandshake(boost::system::error_code ec) {
  if (ec) {
    LOG_ERRO("Handshake error (%s)", ec.message().c_str());
    error_.Set(error_codes::kHandshakeError, "Handshake error");
    RequestEnd();
    return;
  }

  LOG_INFO("Handshake OK");

  BlockingClientBase::AsyncWrite();
}

}  // namespace webcc
