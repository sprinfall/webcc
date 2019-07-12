#include "webcc/connection.h"

#include <utility>

#include "boost/asio/write.hpp"

#include "webcc/connection_pool.h"
#include "webcc/logger.h"
#include "webcc/server.h"

using boost::asio::ip::tcp;

namespace webcc {

Connection::Connection(tcp::socket socket, ConnectionPool* pool,
                       Server* server)
    : socket_(std::move(socket)),
      pool_(pool),
      buffer_(kBufferSize),
      server_(server) {
}

void Connection::Start() {
  request_.reset(new Request{});
  request_parser_.Init(request_.get());
  DoRead();
}

void Connection::Close() {
  LOG_INFO("Close socket...");

  boost::system::error_code ec;
  socket_.close(ec);
  if (ec) {
    LOG_ERRO("Socket close error (%s).", ec.message().c_str());
  }
}

void Connection::SendResponse(ResponsePtr response) {
  assert(response);

  response_ = response;

  if (request_->IsConnectionKeepAlive()) {
    response_->SetHeader(headers::kConnection, "Keep-Alive");
  } else {
    response_->SetHeader(headers::kConnection, "Close");
  }

  response_->Prepare();

  DoWrite();
}

void Connection::SendResponse(Status status) {
  auto response = std::make_shared<Response>(status);

  // According to the testing based on HTTPie (and Chrome), the `Content-Length`
  // header is expected for a response with status like 404 even when the body
  // is empty.
  response->SetBody(std::make_shared<Body>(), true);

  SendResponse(response);
}

void Connection::DoRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&Connection::OnRead, shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void Connection::OnRead(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    // TODO
    if (ec == boost::asio::error::eof) {
      LOG_WARN("Socket read EOF.");
    //} else if (ec == boost::asio::error::operation_aborted) {
    //  LOG_WARN("Socket read aborted.");
    //} else if (ec == boost::asio::error::connection_aborted) {
    //  LOG_WARN("Socket connection aborted.");
    } else {
      LOG_ERRO("Socket read error (%s).", ec.message().c_str());
    }

    if (ec != boost::asio::error::operation_aborted) {
      pool_->Close(shared_from_this());
    }

    return;
  }

  if (!request_parser_.Parse(buffer_.data(), length)) {
    // Bad request.
    // TODO: Always close the connection?
    LOG_ERRO("Failed to parse HTTP request.");
    SendResponse(Status::kBadRequest);
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    DoRead();
    return;
  }

  LOG_VERB("HTTP request:\n%s", request_->Dump().c_str());

  // Enqueue this connection.
  // Some worker thread will handle it later.
  server_->Enqueue(shared_from_this());
}

void Connection::DoWrite() {
  LOG_VERB("HTTP response:\n%s", response_->Dump().c_str());

  // Firstly, write the headers.
  boost::asio::async_write(socket_, response_->GetPayload(),
                           std::bind(&Connection::OnWriteHeaders,
                                     shared_from_this(), std::placeholders::_1,
                                     std::placeholders::_2));
}

void Connection::OnWriteHeaders(boost::system::error_code ec,
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
    boost::asio::async_write(socket_, payload,
                             std::bind(&Connection::OnWriteBody,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
  } else {
    // No more body payload left, we're done.
    OnWriteOK();
  }
}

void Connection::OnWriteBody(boost::system::error_code ec, std::size_t length) {
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
    Shutdown();
    pool_->Close(shared_from_this());
  }
}

void Connection::OnWriteError(boost::system::error_code ec) {
  LOG_ERRO("Socket write error (%s).", ec.message().c_str());

  if (ec != boost::asio::error::operation_aborted) {
    pool_->Close(shared_from_this());
  }
}

// Socket close VS. shutdown:
//   https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
void Connection::Shutdown() {
  LOG_INFO("Shutdown socket...");

  // Initiate graceful connection closure.
  boost::system::error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_ERRO("Socket shutdown error (%s).", ec.message().c_str());
  }
}

}  // namespace webcc
