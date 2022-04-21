#include "webcc/connection_base.h"

#include "boost/asio/write.hpp"

#include "webcc/connection_pool.h"
#include "webcc/logger.h"
#include "webcc/utility.h"  // for utility::HttpDate()

using boost::asio::ip::tcp;
using namespace std::placeholders;

namespace webcc {

ConnectionBase::ConnectionBase(boost::asio::io_context& io_context,
                               ConnectionPool* pool,
                               Queue<ConnectionPtr>* queue,
                               ViewMatcher&& view_matcher,
                               std::size_t buffer_size)
    : pool_(pool),
      queue_(queue),
      view_matcher_(std::move(view_matcher)),
      buffer_(buffer_size) {
}

void ConnectionBase::Close() {
  LOG_INFO("Shutdown and close socket...");

  // Initiate graceful connection closure.
  // Socket close VS. shutdown:
  //   https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
  boost::system::error_code ec;
  GetSocket().shutdown(tcp::socket::shutdown_both, ec);
  if (ec) {
    LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
    ec.clear();
    // Don't return, try to close the socket anywhere.
  }

  GetSocket().close(ec);
  if (ec) {
    LOG_WARN("Socket close error (%s)", ec.message().c_str());
  }

  LOG_INFO("Socket closed");
}

void ConnectionBase::SendResponse(ResponsePtr response, bool no_keep_alive) {
  assert(response != nullptr);

  response_ = response;

  if (!no_keep_alive && request_->IsConnectionKeepAlive()) {
    response_->SetHeader(headers::kConnection, "Keep-Alive");
  } else {
    response_->SetHeader(headers::kConnection, "Close");
  }

  response_->SetHeader(headers::kDate, utility::HttpDate());

  response_->Prepare();

  AsyncWrite();
}

void ConnectionBase::SendResponse(int status, bool no_keep_alive) {
  auto response = std::make_shared<Response>(status);

  // According to the testing based on HTTPie (and Chrome), the Content-Length
  // header is expected for a response with status like 404 even when the body
  // is empty.
  response->SetBody(std::make_shared<Body>(), true);

  SendResponse(response, no_keep_alive);
}

void ConnectionBase::PrepareRequest() {
  request_.reset(new Request{});

  // TODO
  boost::system::error_code ec;
  auto endpoint = GetSocket().remote_endpoint(ec);
  if (!ec) {
    request_->set_address(endpoint.address().to_string());
  }

  request_parser_.Init(request_.get(), view_matcher_);
}

void ConnectionBase::AsyncRead() {
#if WEBCC_STUDY_SERVER_THREADING
  LOG_USER("[%u] AsyncRead()", (unsigned int)this);
#endif

  AsyncReadSome(boost::asio::buffer(buffer_),
                std::bind(&ConnectionBase::OnRead, shared_from_this(), _1, _2));
}

void ConnectionBase::OnRead(boost::system::error_code ec, std::size_t length) {
#if WEBCC_STUDY_SERVER_THREADING
  LOG_USER("[%u] OnRead()", (unsigned int)this);
#endif

  if (ec) {
    if (ec == boost::asio::error::eof) {
      LOG_INFO("Socket read EOF (%s)", ec.message().c_str());
    } else if (ec == boost::asio::error::operation_aborted) {
      // The socket of this connection has been closed.
      // This happens, e.g., when the server was stopped by a signal (Ctrl-C).
      LOG_WARN("Socket operation aborted (%s)", ec.message().c_str());
    } else {
      LOG_ERRO("Socket read error (%s)", ec.message().c_str());
    }

    // Don't try to send any response back.

    if (ec != boost::asio::error::operation_aborted) {
      pool_->Close(shared_from_this());
    }  // else: The socket of this connection has already been closed.

    return;
  }

  if (!request_parser_.Parse(buffer_.data(), length)) {
    LOG_ERRO("Failed to parse request");
    // Send Bad Request (400) to the client and no keep-alive.
    SendResponse(status_codes::kBadRequest, true);
    // Close the socket connection.
    pool_->Close(shared_from_this());
    return;
  }

  if (!request_parser_.finished()) {
    // Continue to read the request.
    AsyncRead();
    return;
  }

  LOG_VERB("Request:\n%s", request_->Dump(log_prefix::kIncoming).c_str());

  // Enqueue this connection once the request has been read.
  // Some worker thread will handle the request later.
  queue_->Push(shared_from_this());
}

void ConnectionBase::AsyncWrite() {
#if WEBCC_STUDY_SERVER_THREADING
  LOG_USER("[%u] AsyncWrite()", (unsigned int)this);
#endif

  LOG_VERB("Response:\n%s", response_->Dump(log_prefix::kOutgoing).c_str());

  AsyncWrite(response_->GetPayload(), std::bind(&ConnectionBase::OnWriteHeaders,
                                                shared_from_this(), _1, _2));
}

void ConnectionBase::OnWriteHeaders(boost::system::error_code ec,
                                std::size_t length) {
#if WEBCC_STUDY_SERVER_THREADING
  LOG_USER("[%u] OnWriteHeaders()", (unsigned int)this);
#endif

  if (ec) {
    HandleWriteError(ec);
  } else {
    // Write the body payload by payload.
    response_->body()->InitPayload();
    AsyncWriteBody();
  }
}

void ConnectionBase::AsyncWriteBody() {
  auto payload = response_->body()->NextPayload();

  if (!payload.empty()) {
    AsyncWrite(payload, std::bind(&ConnectionBase::OnWriteBody,
                                  shared_from_this(), _1, _2));
  } else {
    // No more body payload left, we're done.
    HandleWriteOK();
  }
}

void ConnectionBase::OnWriteBody(boost::system::error_code ec, std::size_t length) {
#if WEBCC_STUDY_SERVER_THREADING
  LOG_USER("[%u] OnWriteBody()", (unsigned int)this);
#endif

  if (ec) {
    HandleWriteError(ec);
  } else {
    AsyncWriteBody();
  }
}

void ConnectionBase::HandleWriteOK() {
  LOG_INFO("Response has been sent back");

  if (request_->IsConnectionKeepAlive()) {
    LOG_INFO("The client asked for a keep-alive connection");
    LOG_INFO("Continue to read the next request");
    PrepareRequest();
    AsyncRead();
  } else {
    pool_->Close(shared_from_this());
  }
}

void ConnectionBase::HandleWriteError(boost::system::error_code ec) {
  LOG_ERRO("Socket write error (%s)", ec.message().c_str());

  if (ec != boost::asio::error::operation_aborted) {
    pool_->Close(shared_from_this());
  }
}

}  // namespace webcc
