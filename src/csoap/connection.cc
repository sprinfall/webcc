#include "csoap/connection.h"
#include <vector>

#include "boost/asio/write.hpp"

#include "csoap/connection_manager.h"
#include "csoap/http_request_handler.h"

namespace csoap {

Connection::Connection(boost::asio::ip::tcp::socket socket,
                       ConnectionManager& manager,
                       HttpRequestHandler& handler)
    : socket_(std::move(socket))
    , connection_manager_(manager)
    , request_handler_(handler)
    , request_parser_(&request_) {
}

void Connection::Start() {
  DoRead();
}

void Connection::Stop() {
  socket_.close();
}

void Connection::DoRead() {
  auto handler = std::bind(&Connection::HandleRead,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);
  socket_.async_read_some(boost::asio::buffer(buffer_), handler);
}

void Connection::DoWrite() {
  auto handler = std::bind(&Connection::HandleWrite,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);
  boost::asio::async_write(socket_, response_.ToBuffers(), handler);
}

void Connection::HandleRead(boost::system::error_code ec,
                            std::size_t bytes_transferred) {
  if (ec) {
    if (ec != boost::asio::error::operation_aborted) {
      connection_manager_.Stop(shared_from_this());
    }
    return;
  }

  ErrorCode error = request_parser_.Parse(buffer_.data(), bytes_transferred);

  if (error != kNoError) {
    // Bad request.
    response_ = HttpResponse::Fault(HttpStatus::BAD_REQUEST);
    DoWrite();

    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    DoRead();
    return;
  }

  // Handle request.
  request_handler_.HandleRequest(request_, response_);

  // Send back the response.
  DoWrite();
}

void Connection::HandleWrite(boost::system::error_code ec,
                             size_t bytes_transferred) {
  if (!ec) {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                     ignored_ec);
  }

  if (ec != boost::asio::error::operation_aborted) {
    connection_manager_.Stop(shared_from_this());
  }
}

}  // namespace csoap
