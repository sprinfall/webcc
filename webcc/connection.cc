#include "webcc/connection.h"

#include <utility>

#include "asio/write.hpp"

#include "webcc/connection_pool.h"
#include "webcc/logger.h"

using asio::ip::tcp;

namespace webcc {

Connection::Connection(tcp::socket socket, ConnectionPool* pool,
                       Queue<ConnectionPtr>* queue, ViewMatcher&& view_matcher)
    : socket_(std::move(socket)), pool_(pool), queue_(queue),
      view_matcher_(std::move(view_matcher)), buffer_(kBufferSize) {
}

void Connection::Start() {
  request_.reset(new Request{});

  std::error_code ec;
  auto endpoint = socket_.remote_endpoint(ec);
  if (!ec) {
    request_->set_ip(endpoint.address().to_string());
  }

  request_parser_.Init(request_.get(), view_matcher_);
  DoRead();
}

void Connection::Close() {
  LOG_INFO("Shutdown socket...");

  // Initiate graceful connection closure.
  // Socket close VS. shutdown:
  //   https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
  std::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_WARN("Socket shutdown error (%s).", ec.message().c_str());
    ec.clear();
    // Don't return, try to close the socket anywhere.
  }

  LOG_INFO("Close socket...");

  socket_.close(ec);

  if (ec) {
    LOG_ERRO("Socket close error (%s).", ec.message().c_str());
  }
}

void Connection::SendResponse(ResponsePtr response, bool no_keep_alive) {
  assert(response);

  response_ = response;

  if (!no_keep_alive && request_->IsConnectionKeepAlive()) {
    response_->SetHeader(headers::kConnection, "Keep-Alive");
  } else {
    response_->SetHeader(headers::kConnection, "Close");
  }

  response_->Prepare();

  DoWrite();
}

void Connection::SendResponse(Status status, bool no_keep_alive) {
  auto response = std::make_shared<Response>(status);

  // According to the testing based on HTTPie (and Chrome), the `Content-Length`
  // header is expected for a response with status like 404 even when the body
  // is empty.
  response->SetBody(std::make_shared<Body>(), true);

  SendResponse(response, no_keep_alive);
}

void Connection::DoRead() {
  socket_.async_read_some(asio::buffer(buffer_),
                          std::bind(&Connection::OnRead, shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void Connection::OnRead(std::error_code ec, std::size_t length) {
  if (ec) {
    if (ec == asio::error::eof) {
      LOG_INFO("Socket read EOF (%s).", ec.message().c_str());
    } else if (ec == asio::error::operation_aborted) {
      // The socket of this connection has been closed.
      // This happens, e.g., when the server was stopped by a signal (Ctrl-C).
      LOG_WARN("Socket operation aborted (%s).", ec.message().c_str());
    } else {
      LOG_ERRO("Socket read error (%s).", ec.message().c_str());
    }

    // Don't try to send any response back.

    if (ec != asio::error::operation_aborted) {
      pool_->Close(shared_from_this());
    }  // else: The socket of this connection has already been closed.

    return;
  }

  if (!request_parser_.Parse(buffer_.data(), length)) {
    LOG_ERRO("Failed to parse HTTP request.");
    // Send Bad Request (400) to the client and no Keep-Alive.
    SendResponse(Status::kBadRequest, true);
    // Close the socket connection.
    pool_->Close(shared_from_this());
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    DoRead();
    return;
  }

  LOG_VERB("HTTP request:\n%s", request_->Dump().c_str());

  // Enqueue this connection once the request has been read.
  // Some worker thread will handle the request later.
  queue_->Push(shared_from_this());
}

void Connection::DoWrite() {
  LOG_VERB("HTTP response:\n%s", response_->Dump().c_str());

  // Firstly, write the headers.
  asio::async_write(socket_, response_->GetPayload(),
                           std::bind(&Connection::OnWriteHeaders,
                                     shared_from_this(), std::placeholders::_1,
                                     std::placeholders::_2));
}

void Connection::OnWriteHeaders(std::error_code ec,
                                std::size_t length) {
  if (ec) {
    OnWriteError(ec);
  } else {
    // Write the body payload by payload.
    response_->body()->InitPayload();
    DoWriteBody();
  }
}

void Connection::DoWriteBody() {
  auto payload = response_->body()->NextPayload();

  if (!payload.empty()) {
    asio::async_write(socket_, payload,
                             std::bind(&Connection::OnWriteBody,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
  } else {
    // No more body payload left, we're done.
    OnWriteOK();
  }
}

void Connection::OnWriteBody(std::error_code ec, std::size_t length) {
  if (ec) {
    OnWriteError(ec);
  } else {
    DoWriteBody();
  }
}

void Connection::OnWriteOK() {
  LOG_INFO("Response has been sent back.");

  if (request_->IsConnectionKeepAlive()) {
    LOG_INFO("The client asked for a keep-alive connection.");
    LOG_INFO("Continue to read the next request...");
    Start();
  } else {
    pool_->Close(shared_from_this());
  }
}

void Connection::OnWriteError(std::error_code ec) {
  LOG_ERRO("Socket write error (%s).", ec.message().c_str());

  if (ec != asio::error::operation_aborted) {
    pool_->Close(shared_from_this());
  }
}

}  // namespace webcc
