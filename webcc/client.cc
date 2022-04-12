#include "webcc/client.h"

#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

namespace webcc {

void Client::AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                        RWHandler&& handler) {
  boost::asio::async_write(socket_, buffers, std::move(handler));
}

void Client::AsyncReadSome(boost::asio::mutable_buffer buffer,
                           RWHandler&& handler) {
  socket_.async_read_some(buffer, std::move(handler));
}

}  // namespace webcc
