#include "webcc/ssl_connection.h"

#include "boost/asio/write.hpp"
#include "boost/asio/ssl.hpp"

#include "webcc/connection_pool.h"
#include "webcc/logger.h"

namespace webcc {

using namespace std::placeholders;
namespace ssl = boost::asio::ssl;

void SslConnection::Start() {
  ssl_stream_.async_handshake(ssl::stream_base::server,
                              std::bind(&SslConnection::OnHandshake, this, _1));
}

void SslConnection::AsyncWrite(
    const std::vector<boost::asio::const_buffer>& buffers,
    RWHandler&& handler) {
  boost::asio::async_write(ssl_stream_, buffers, std::move(handler));
}

void SslConnection::AsyncReadSome(boost::asio::mutable_buffer buffer,
                                  RWHandler&& handler) {
  ssl_stream_.async_read_some(buffer, std::move(handler));
}

void SslConnection::OnHandshake(boost::system::error_code ec) {
  if (ec) {
    LOG_ERRO("Handshake error (%s)", ec.message().c_str());
    pool_->Close(shared_from_this());
    return;
  }

  PrepareRequest();
  AsyncRead();
}

}  // namespace webcc
