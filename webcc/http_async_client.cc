#include "webcc/http_async_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

HttpAsyncClient::HttpAsyncClient(boost::asio::io_context& io_context,
                                 std::size_t buffer_size)
    : HttpAsyncClientBase(io_context, buffer_size),
      socket_(io_context) {
}

void HttpAsyncClient::SocketAsyncConnect(const Endpoints& endpoints,
                                         ConnectHandler&& handler) {
  boost::asio::async_connect(socket_, endpoints, std::move(handler));
}

void HttpAsyncClient::SocketAsyncWrite(WriteHandler&& handler) {
  boost::asio::async_write(socket_, request_->ToBuffers(), std::move(handler));
}

void HttpAsyncClient::SocketAsyncReadSome(ReadHandler&& handler) {
  socket_.async_read_some(boost::asio::buffer(buffer_), std::move(handler));
}

void HttpAsyncClient::SocketClose(boost::system::error_code* ec) {
  socket_.close(*ec);
}

}  // namespace webcc
