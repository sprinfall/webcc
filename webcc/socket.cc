#include "webcc/socket.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/core/ignore_unused.hpp"

#include "webcc/logger.h"

using boost::asio::ip::tcp;

namespace webcc {

Socket::Socket(boost::asio::io_context& io_context) : socket_(io_context) {
}

void Socket::AsyncConnect(const std::string& host, const Endpoints& endpoints,
                          ConnectHandler&& handler) {
  boost::ignore_unused(host);

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

}  // namespace webcc
