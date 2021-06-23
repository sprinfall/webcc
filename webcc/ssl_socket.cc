#include "webcc/ssl_socket.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

using namespace std::placeholders;

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace webcc {

SslSocket::SslSocket(boost::asio::io_context& io_context,
                     ssl::context& ssl_context)
    : ssl_stream_(io_context, ssl_context) {
}

void SslSocket::AsyncConnect(const std::string& host,
                             const Endpoints& endpoints,
                             ConnectHandler&& handler) {
  connect_handler_ = std::move(handler);

  // Set SNI (server name indication) host name.
  // Many hosts need this to handshake successfully (e.g., google.com).
  // Inspired by Boost.Beast.
  if (!SSL_set_tlsext_host_name(ssl_stream_.native_handle(), host.c_str())) {
    // TODO: Call ERR_get_error() to get error.
    LOG_ERRO("Failed to set SNI host name for SSL");
  }

  // Modes `ssl::verify_fail_if_no_peer_cert` and `ssl::verify_client_once` are
  // for server only. `ssl::verify_none` is not secure.
  // See: https://stackoverflow.com/a/12621528
  ssl_stream_.set_verify_mode(ssl::verify_peer);

  // ssl::host_name_verification has been added since Boost 1.73 to replace
  // ssl::rfc2818_verification.
#if BOOST_VERSION < 107300
  ssl_stream_.set_verify_callback(ssl::rfc2818_verification(host));
#else
  ssl_stream_.set_verify_callback(ssl::host_name_verification(host));
#endif  // BOOST_VERSION < 107300

  boost::asio::async_connect(ssl_stream_.lowest_layer(), endpoints,
                             std::bind(&SslSocket::OnConnect, this, _1, _2));
}

void SslSocket::AsyncWrite(const Payload& payload, WriteHandler&& handler) {
  boost::asio::async_write(ssl_stream_, payload, std::move(handler));
}

void SslSocket::AsyncReadSome(ReadHandler&& handler,
                              std::vector<char>* buffer) {
  ssl_stream_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

bool SslSocket::Shutdown() {
  boost::system::error_code ec;

  ssl_stream_.lowest_layer().cancel(ec);

  // Shutdown SSL
  // TODO: Use async_shutdown()?
  ssl_stream_.shutdown(ec);

  if (ec == boost::asio::error::eof) {
    // See: https://stackoverflow.com/a/25703699
    ec = {};
  }

  if (ec) {
    LOG_WARN("SSL shutdown error (%s)", ec.message().c_str());
    return false;
  }

  // Shutdown TCP
  // TODO: Not sure if this is necessary?
  ssl_stream_.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
    return false;
  }

  return true;
}

bool SslSocket::Close() {
  boost::system::error_code ec;
  ssl_stream_.lowest_layer().close(ec);

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

  ssl_stream_.async_handshake(ssl::stream_base::client,
                              [this](boost::system::error_code ec) {
    if (ec) {
      LOG_ERRO("Handshake error (%s)", ec.message().c_str());
    }

    connect_handler_(ec, std::move(endpoint_));
  });
}

}  // namespace webcc
