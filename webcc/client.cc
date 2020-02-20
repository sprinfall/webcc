#include "webcc/client.h"

#include "webcc/logger.h"

using asio::ip::tcp;

namespace webcc {

Client::Client()
    : timer_(io_context_),
      ssl_verify_(true),
      buffer_size_(kBufferSize),
      timeout_(kMaxReadSeconds),
      closed_(false),
      timer_canceled_(false) {
}

Error Client::Request(RequestPtr request, bool connect, bool stream) {
  closed_ = false;
  timer_canceled_ = false;
  error_ = Error{};

  response_.reset(new Response{});
  response_parser_.Init(response_.get(), stream);

  if (buffer_.size() != buffer_size_) {
    LOG_VERB("Resize buffer: %u -> %u.", buffer_.size(), buffer_size_);
    buffer_.resize(buffer_size_);
  }

  // Response to HEAD could also have Content-Length.
  // Set this flag to skip the reading and parsing of the body.
  // The test against HttpBin.org shows that:
  //   - If request.Accept-Encoding is "gzip, deflate", the response won't
  //     have Content-Length;
  //   - If request.Accept-Encoding is "identity", the response will have
  //     Content-Length.
  if (request->method() == methods::kHead) {
    response_parser_.set_ignroe_body(true);
  } else {
    // Reset in case the connection is persistent.
    response_parser_.set_ignroe_body(false);
  }

  io_context_.restart();

  if (connect) {
    // No existing socket connection was specified, create a new one.
    Connect(request);

    if (error_) {
      return error_;
    }
  }

  WriteRequest(request);

  if (error_) {
    return error_;
  }

  ReadResponse();

  return error_;
}

void Client::Close() {
  if (closed_) {
    return;
  }

  closed_ = true;

  LOG_INFO("Close socket...");

  socket_->Close();
}

void Client::Connect(RequestPtr request) {
  if (request->url().scheme() == "https") {
#if WEBCC_ENABLE_SSL
    socket_.reset(new SslSocket{ io_context_, ssl_verify_ });
    DoConnect(request, "443");
#else
    LOG_ERRO("SSL/HTTPS support is not enabled.");
    error_.Set(Error::kSyntaxError, "SSL/HTTPS is not supported");
#endif  // WEBCC_ENABLE_SSL
  } else {
    socket_.reset(new Socket{ io_context_ });
    DoConnect(request, "80");
  }
}

void Client::DoConnect(RequestPtr request, const std::string& default_port) {
  tcp::resolver resolver(io_context_);

  std::string port = request->port();
  if (port.empty()) {
    port = default_port;
  }

  std::error_code ec;
  auto endpoints = resolver.resolve(tcp::v4(), request->host(), port, ec);

  if (ec) {
    LOG_ERRO("Host resolve error (%s): %s, %s.", ec.message().c_str(),
             request->host().c_str(), port.c_str());
    error_.Set(Error::kResolveError, "Host resolve error");
    return;
  }

  LOG_VERB("Connect to server...");

  // Use sync API directly since we don't need timeout control.

  if (!socket_->Connect(request->host(), endpoints)) {
    error_.Set(Error::kConnectError, "Endpoint connect error");
    Close();
    return;
  }

  LOG_VERB("Socket connected.");
}

void Client::WriteRequest(RequestPtr request) {
  LOG_VERB("HTTP request:\n%s", request->Dump().c_str());

  // NOTE:
  // It doesn't make much sense to set a timeout for socket write.
  // I find that it's almost impossible to simulate a situation in the server
  // side to test this timeout.

  // Use sync API directly since we don't need timeout control.

  std::error_code ec;

  if (socket_->Write(request->GetPayload(), &ec)) {
    // Write request body.
    auto body = request->body();
    body->InitPayload();
    for (auto p = body->NextPayload(true); !p.empty();
         p = body->NextPayload(true)) {
      if (!socket_->Write(p, &ec)) {
        break;
      }
    }
  }

  if (ec) {
    LOG_ERRO("Socket write error (%s).", ec.message().c_str());
    Close();
    error_.Set(Error::kSocketWriteError, "Socket write error");
  }

  LOG_INFO("Request sent.");
}

void Client::ReadResponse() {
  LOG_VERB("Read response (timeout: %ds)...", timeout_);

  DoReadResponse();

  if (!error_) {
    LOG_VERB("HTTP response:\n%s", response_->Dump().c_str());
  }
}

void Client::DoReadResponse() {
  std::error_code ec = asio::error::would_block;
  std::size_t length = 0;

  // The read handler.
  auto handler = [&ec, &length](std::error_code inner_ec,
                                std::size_t inner_length) {
    ec = inner_ec;
    length = inner_length;
  };

  while (true) {
    ec = asio::error::would_block;
    length = 0;

    socket_->AsyncReadSome(std::move(handler), &buffer_);

    // Start the timer.
    DoWaitTimer();

    // Block until the asynchronous operation has completed.
    do {
      io_context_.run_one();
    } while (ec == asio::error::would_block);

    // Stop the timer.
    CancelTimer();

    // The error normally is caused by timeout. See OnTimer().
    if (ec || length == 0) {
      Close();
      error_.Set(Error::kSocketReadError, "Socket read error");
      LOG_ERRO("Socket read error (%s).", ec.message().c_str());
      break;
    }

    LOG_INFO("Read data, length: %u.", length);

    // Parse the piece of data just read.
    if (!response_parser_.Parse(buffer_.data(), length)) {
      Close();
      error_.Set(Error::kParseError, "HTTP parse error");
      LOG_ERRO("Failed to parse the HTTP response.");
      break;
    }

    if (response_parser_.finished()) {
      // Stop trying to read once all content has been received, because
      // some servers will block extra call to read_some().

      if (response_->IsConnectionKeepAlive()) {
        // Close the timer but keep the socket connection.
        LOG_INFO("Keep the socket connection alive.");
      } else {
        Close();
      }

      // Stop reading.
      LOG_INFO("Finished to read the HTTP response.");
      break;
    }
  }
}

void Client::DoWaitTimer() {
  LOG_VERB("Wait timer asynchronously.");
  timer_.expires_after(std::chrono::seconds(timeout_));
  timer_.async_wait(std::bind(&Client::OnTimer, this, std::placeholders::_1));
}

void Client::OnTimer(std::error_code ec) {
  LOG_VERB("On timer.");

  // timer_.cancel() was called.
  if (ec == asio::error::operation_aborted) {
    LOG_VERB("Timer canceled.");
    return;
  }

  if (closed_) {
    LOG_VERB("Socket has been closed.");
    return;
  }

  if (timer_.expiry() <= asio::steady_timer::clock_type::now()) {
    // The deadline has passed. The socket is closed so that any outstanding
    // asynchronous operations are canceled.
    LOG_WARN("HTTP client timed out.");
    error_.set_timeout(true);
    Close();
    return;
  }

  // Put the actor back to sleep.
  DoWaitTimer();
}

void Client::CancelTimer() {
  if (timer_canceled_) {
    return;
  }

  LOG_INFO("Cancel timer...");
  timer_.cancel();

  timer_canceled_ = true;
}

}  // namespace webcc
