#include "webcc/ssl_client.h"

#include "boost/algorithm/string.hpp"

#include "webcc/ssl_socket.h"

namespace webcc {

SslClient::SslClient(boost::asio::io_context& io_context,
                     boost::asio::ssl::context& ssl_context)
    : Client(io_context), ssl_context_(ssl_context) {
}

void SslClient::AsyncConnect() {
  if (boost::iequals(request_->url().scheme(), "https")) {
    socket_.reset(new SslSocket{ io_context_, ssl_context_ });
    AsyncResolve("443");
  } else {
    Client::AsyncConnect();
  }
}

}  // namespace webcc
