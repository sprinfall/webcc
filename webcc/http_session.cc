#include "webcc/http_session.h"

#include <utility>  // for move()

#include "boost/asio/write.hpp"

#include "webcc/http_request_handler.h"
#include "webcc/logger.h"

using boost::asio::ip::tcp;

namespace webcc {

HttpSession::HttpSession(tcp::socket socket, HttpRequestHandler* handler)
    : socket_(std::move(socket)),
      buffer_(kBufferSize),
      request_handler_(handler),
      request_parser_(&request_) {
}

void HttpSession::Start() {
  DoRead();
}

void HttpSession::Close() {
  LOG_INFO("Close socket...");

  boost::system::error_code ec;
  socket_.close(ec);
  if (ec) {
    LOG_ERRO("Socket close error (%s).", ec.message().c_str());
  }
}

void HttpSession::SetResponseContent(std::string&& content,
                                     const std::string& media_type,
                                     const std::string& charset) {
  response_.SetContent(std::move(content), true);
  response_.SetContentType(media_type, charset);
}

void HttpSession::SendResponse(http::Status status) {
  response_.set_status(status);
  response_.Make();
  DoWrite();
}

void HttpSession::DoRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpSession::OnRead, shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpSession::OnRead(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    LOG_ERRO("Socket read error (%s).", ec.message().c_str());
    if (ec != boost::asio::error::operation_aborted) {
      Close();
    }
    return;
  }

  if (!request_parser_.Parse(buffer_.data(), length)) {
    // Bad request.
    LOG_ERRO("Failed to parse HTTP request.");
    response_ = HttpResponse::Fault(http::Status::kBadRequest);
    DoWrite();
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    DoRead();
    return;
  }

  LOG_VERB("HTTP request:\n%s", request_.Dump(4, "> ").c_str());

  // Enqueue this session.
  // Some worker thread will handle it later.
  request_handler_->Enqueue(shared_from_this());
}

void HttpSession::DoWrite() {
  LOG_VERB("HTTP response:\n%s", response_.Dump(4, "> ").c_str());

  boost::asio::async_write(socket_, response_.ToBuffers(),
                           std::bind(&HttpSession::OnWrite, shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

// NOTE:
// This write handler will be called from main thread (the thread calling
// io_context.run), even though AsyncWrite() is invoked by worker threads.
// This is ensured by Asio.
void HttpSession::OnWrite(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    LOG_ERRO("Socket write error (%s).", ec.message().c_str());

    if (ec != boost::asio::error::operation_aborted) {
      Close();
    }
  } else {
    LOG_INFO("Response has been sent back, length: %u.", length);

    Shutdown();
    Close();  // Necessary even after shutdown!
  }
}

// Socket close VS. shutdown:
//   https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
void HttpSession::Shutdown() {
  LOG_INFO("Shutdown socket...");

  // Initiate graceful connection closure.
  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_ERRO("Socket shutdown error (%s).", ec.message().c_str());
  }
}

}  // namespace webcc
