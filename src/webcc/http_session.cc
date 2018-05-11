#include "webcc/http_session.h"

#include <vector>

#include "boost/asio/write.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "webcc/logger.h"
#include "webcc/http_request_handler.h"

namespace webcc {

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket,
                         HttpRequestHandler* handler)
    : socket_(std::move(socket))
    , request_handler_(handler)
    , request_parser_(&request_) {
}

void HttpSession::Start() {
  AsyncRead();
}

void HttpSession::Close() {
  boost::system::error_code ec;
  socket_.close(ec);
}

void HttpSession::SetResponseContent(std::string&& content,
                                     const std::string& content_type) {
  response_.SetContent(std::move(content));
  response_.SetContentType(content_type);
}

void HttpSession::SendResponse(HttpStatus::Enum status) {
  response_.set_status(status);
  AsyncWrite();
}

void HttpSession::AsyncRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpSession::ReadHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpSession::ReadHandler(boost::system::error_code ec,
                              std::size_t length) {
  if (ec) {
    if (ec != boost::asio::error::operation_aborted) {
      Close();
    }
    return;
  }

  Error error = request_parser_.Parse(buffer_.data(), length);

  if (error != kNoError) {
    // Bad request.
    response_ = HttpResponse::Fault(HttpStatus::kBadRequest);
    AsyncWrite();
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    AsyncRead();
    return;
  }

  // Enqueue this session.
  // Some worker thread will handle it later.
  // And DoWrite() will be called in the worker thread.
  request_handler_->Enqueue(shared_from_this());
}

void HttpSession::AsyncWrite() {
  boost::asio::async_write(socket_,
                           response_.ToBuffers(),
                           std::bind(&HttpSession::WriteHandler,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

// NOTE:
// This write handler will be called from main thread (the thread calling
// io_context.run), even though DoWrite() is invoked by worker threads. This is
// ensured by Asio.
void HttpSession::WriteHandler(boost::system::error_code ec,
                               std::size_t length) {
  if (!ec) {
    LOG_INFO("Response has been sent back.");

    // Initiate graceful connection closure.
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  } else {
    LOG_ERRO("Sending response error: %s", ec.message().c_str());

    if (ec != boost::asio::error::operation_aborted) {
      Close();
    }
  }
}

}  // namespace webcc
