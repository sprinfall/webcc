#include "webcc/http_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

namespace webcc {

HttpClient::HttpClient(std::size_t buffer_size)
    : HttpClientBase(buffer_size), socket_(io_context_) {
}

void HttpClient::SocketConnect(const Endpoints& endpoints,
                               boost::system::error_code* ec) {
  boost::asio::connect(socket_, endpoints, *ec);
}

void HttpClient::SocketWrite(const HttpRequest& request,
                             boost::system::error_code* ec) {
  boost::asio::write(socket_, request.ToBuffers(), *ec);
}

void HttpClient::SocketAsyncReadSome(ReadHandler&& handler) {
  socket_.async_read_some(boost::asio::buffer(buffer_), std::move(handler));
}

void HttpClient::SocketClose(boost::system::error_code* ec) {
  socket_.close(*ec);
}

}  // namespace webcc
