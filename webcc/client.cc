#include "webcc/client.h"

#include "boost/asio/write.hpp"

#include "webcc/logger.h"

namespace webcc {

bool Client::Close() {
  if (connected_) {
    boost::system::error_code ec;

    // Cancel any pending async operations on the socket.
    GetSocket().cancel(ec);
    if (ec) {
      LOG_WARN("Socket cancel error (%s)", ec.message().c_str());
      ec.clear();
    }
  }

  return ClientBase::Close();
}

void Client::AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                        AsyncRWHandler&& handler) {
  boost::asio::async_write(socket_, buffers, std::move(handler));
}

void Client::AsyncReadSome(boost::asio::mutable_buffer buffer,
                           AsyncRWHandler&& handler) {
  socket_.async_read_some(buffer, std::move(handler));
}

}  // namespace webcc
