#include "csoap/http_session.h"

#include <vector>
#if CSOAP_DEBUG_OUTPUT
#include <iostream>
#endif

#include "boost/asio/write.hpp"

#include "csoap/http_request_handler.h"

namespace csoap {

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket,
                         HttpRequestHandler* handler)
    : socket_(std::move(socket))
    , request_handler_(handler)
    , request_parser_(&request_) {
}

void HttpSession::Start() {
  DoRead();
}

void HttpSession::Stop() {
  socket_.close();
}

void HttpSession::SetResponseContent(const std::string& content_type,
                                     std::size_t content_length,
                                     std::string&& content) {
  response_.SetContentType(content_type);
  response_.SetContentLength(content.length());
  response_.set_content(std::move(content));
}

void HttpSession::SendResponse() {
  DoWrite();
}

void HttpSession::DoRead() {
  auto handler = std::bind(&HttpSession::HandleRead,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);
  socket_.async_read_some(boost::asio::buffer(buffer_), handler);
}

void HttpSession::DoWrite() {
  auto handler = std::bind(&HttpSession::HandleWrite,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);
  boost::asio::async_write(socket_, response_.ToBuffers(), handler);
}

void HttpSession::HandleRead(boost::system::error_code ec,
                             std::size_t length) {
  if (ec) {
    if (ec != boost::asio::error::operation_aborted) {
      Stop();
    }
    return;
  }

  Error error = request_parser_.Parse(buffer_.data(), length);

  if (error != kNoError) {
    // Bad request.
    response_ = HttpResponse::Fault(HttpStatus::kBadRequest);
    DoWrite();
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    DoRead();
    return;
  }

  // Enqueue this session.
  // Some worker thread will handle it later.
  // And DoWrite() will be called in the worker thread.
  request_handler_->Enqueue(shared_from_this());
}

// NOTE:
// This write handler will be called from main thread (the thread calling
// io_context.run), even though DoWrite() is invoked by worker threads. This is
// ensured by Asio.
void HttpSession::HandleWrite(boost::system::error_code ec,
                              size_t length) {
#if CSOAP_DEBUG_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
  std::cout << "Response has been sent back (thread: " << thread_id << ")\n";
#endif

  if (!ec) {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  if (ec != boost::asio::error::operation_aborted) {
    Stop();
  }
}

}  // namespace csoap
