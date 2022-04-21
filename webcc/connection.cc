#include "webcc/connection.h"

#include "boost/asio/write.hpp"

namespace webcc {

void Connection::AsyncWrite(
    const std::vector<boost::asio::const_buffer>& buffers,
    RWHandler&& handler) {
  boost::asio::async_write(socket_, buffers, std::move(handler));
}

void Connection::AsyncReadSome(boost::asio::mutable_buffer buffer,
                               RWHandler&& handler) {
  socket_.async_read_some(buffer, std::move(handler));
}

}  // namespace webcc
