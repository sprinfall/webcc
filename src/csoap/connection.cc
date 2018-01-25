#include "csoap/connection.h"

#include <vector>
#if CSOAP_ENABLE_OUTPUT
#include <iostream>
#endif

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
                            std::size_t length) {
  if (ec) {
    if (ec != boost::asio::error::operation_aborted) {
      connection_manager_.Stop(shared_from_this());
    }
    return;
  }

  Error error = request_parser_.Parse(buffer_.data(), length);

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

  // Enqueue this connection.
  // Some worker thread will handle it later.
  // And DoWrite() will be called in the worker thread.
  request_handler_.Enqueue(shared_from_this());
}

// NOTE:
// This write handler will be called from main thread (the thread calling
// io_context.run), even though DoWrite() is invoked by worker threads. This is
// ensured by Asio.
void Connection::HandleWrite(boost::system::error_code ec,
                             size_t length) {
#if CSOAP_ENABLE_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
  std::cout << "Response has been sent back (thread: " << thread_id << ")\n";
#endif

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
