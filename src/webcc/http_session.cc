#include "webcc/http_session.h"

#include <vector>
#if WEBCC_DEBUG_OUTPUT
#include <iostream>
#endif

#include "boost/asio/write.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "webcc/http_request_handler.h"

namespace webcc {

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket,
                         HttpRequestHandler* handler)
    : socket_(std::move(socket))
    , request_handler_(handler)
    , request_parser_(&request_) {
}

HttpSession::~HttpSession() {
}

void HttpSession::Start(long timeout_seconds) {
  if (timeout_seconds > 0) {
    // Create timer only necessary.
    boost::asio::io_context& ioc = socket_.get_executor().context();
    timer_.reset(new boost::asio::deadline_timer(ioc));

    timer_->expires_from_now(boost::posix_time::seconds(timeout_seconds));

    timer_->async_wait(std::bind(&HttpSession::HandleTimer,
                                 shared_from_this(),
                                 std::placeholders::_1));
  }
  
  DoRead();
}

void HttpSession::Stop() {
  CancelTimer();

  boost::system::error_code ignored_ec;
  socket_.close(ignored_ec);
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
    } else {
      CancelTimer();
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
                              std::size_t length) {
#if WEBCC_DEBUG_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
#endif

  if (!ec) {
    CancelTimer();

    // Initiate graceful connection closure.
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

#if WEBCC_DEBUG_OUTPUT
    std::cout << "Response has been sent back (thread: " << thread_id << ")\n";
#endif
  } else {
#if WEBCC_DEBUG_OUTPUT
    std::cout << "(thread: " << thread_id << ") Sending response error: "
      << ec.message()
      << std::endl;
#endif

    if (ec == boost::asio::error::operation_aborted) {
      CancelTimer();
    } else {
      Stop();
    }
  }
}

void HttpSession::HandleTimer(boost::system::error_code ec) {
  std::cout << "HandleTimer: ";

  if (!ec) {
    if (socket_.is_open()) {
      std::cout << "socket is open, close it.\n";
      socket_.close();
    } else {
      std::cout << "socket is not open.\n";
    }
  } else {
    if (ec == boost::asio::error::operation_aborted) {
      std::cout << "Timer aborted\n";
    }
  }
}

void HttpSession::CancelTimer() {
  if (timer_) {
    // The wait handler will be invoked with the operation_aborted error code.
    boost::system::error_code ec;
    timer_->cancel(ec);
  }
}

}  // namespace webcc
