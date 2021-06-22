#include "webcc/client.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"

using boost::asio::ip::tcp;
using namespace std::placeholders;

namespace webcc {

#if WEBCC_ENABLE_SSL

Client::Client(boost::asio::io_context& io_context,
               boost::asio::ssl::context& ssl_context)
    : io_context_(io_context),
      ssl_context_(ssl_context),
      resolver_(io_context),
      deadline_timer_(io_context) {
}

#else

Client::Client(boost::asio::io_context& io_context)
    : io_context_(io_context),
      resolver_(io_context),
      deadline_timer_(io_context) {
}

#endif  // WEBCC_ENABLE_SSL

Error Client::Request(RequestPtr request, bool stream) {
  LOG_VERB("Request begin");

  request_finished_ = false;
  error_ = Error{};

  request_ = request;

  length_read_ = 0;
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
  if (request_->method() == methods::kHead) {
    response_parser_.set_ignore_body(true);
  } else {
    // Reset in case the connection is persistent.
    response_parser_.set_ignore_body(false);
  }

  if (!connected_) {
    AsyncConnect();
  } else {
    AsyncWrite();
  }

  // Wait for the request to be finished.
  std::unique_lock<std::mutex> response_lock{ request_mutex_ };
  request_cv_.wait(response_lock, [=] { return request_finished_; });

  LOG_VERB("Request end");

  return error_;
}

void Client::Close() {
  DoClose();

  // Don't call FinishRequest() from here! It will be called in the handler
  // OnXxx with `error::operation_aborted`.
}

void Client::DoClose() {
  if (connected_) {
    connected_ = false;
    if (socket_) {
      LOG_VERB("Shutdown & close socket");
      socket_->Shutdown();
      socket_->Close();
    }
    LOG_INFO("Socket closed");
  } else {
    // TODO: resolver_.cancel() ?
    if (socket_) {
      LOG_INFO("Close socket");
      socket_->Close();
    }
  }
}

void Client::AsyncConnect() {
  if (boost::iequals(request_->url().scheme(), "http")) {
    socket_.reset(new Socket{ io_context_ });
    AsyncResolve("80");
    return;
  }

#if WEBCC_ENABLE_SSL
  if (boost::iequals(request_->url().scheme(), "https")) {
    socket_.reset(new SslSocket{ io_context_, ssl_context_ });
    AsyncResolve("443");
    return;
  }
#endif  // WEBCC_ENABLE_SSL

  LOG_ERRO("URL scheme (%s) is not supported",
           request_->url().scheme().c_str());
  error_.Set(Error::kSyntaxError, "URL scheme not supported");
  FinishRequest();
}

void Client::AsyncResolve(string_view default_port) {
  std::string port = request_->port();
  if (port.empty()) {
    port = ToString(default_port);
  }

  LOG_VERB("Resolve host (%s)", request_->host().c_str());

  // The protocol depends on the `host`, both V4 and V6 are supported.
  resolver_.async_resolve(request_->host(), port,
                          std::bind(&Client::OnResolve, this, _1, _2));
}

void Client::OnResolve(boost::system::error_code ec,
                       tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Host resolve error (%s)", ec.message().c_str());
    error_.Set(Error::kResolveError, "Host resolve error");
    FinishRequest();
    return;
  }

  LOG_VERB("Connect socket");

  AsyncWaitDeadlineTimer(connect_timeout_);

  socket_->AsyncConnect(request_->host(), endpoints,
                        std::bind(&Client::OnConnect, this, _1, _2));
}

void Client::OnConnect(boost::system::error_code ec, tcp::endpoint) {
  LOG_VERB("On connect");

  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or DoClose().
      LOG_WARN("Connect operation aborted");
    } else {
      LOG_INFO("Connect error");
      // No need to close socket since no async operation is on it.
      //   socket_->Close();
    }

    error_.Set(Error::kConnectError, "Socket connect error");
    FinishRequest();
    return;
  }

  LOG_INFO("Socket connected");

  connected_ = true;

  AsyncWrite();
}

void Client::AsyncWrite() {
  LOG_VERB("Request:\n%s", request_->Dump().c_str());

  socket_->AsyncWrite(request_->GetPayload(),
                      std::bind(&Client::OnWrite, this, _1, _2));
}

void Client::OnWrite(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  request_->body()->InitPayload();

  AsyncWriteBody();
}

void Client::AsyncWriteBody() {
  auto p = request_->body()->NextPayload(true);

  if (!p.empty()) {
    socket_->AsyncWrite(p, std::bind(&Client::OnWriteBody, this, _1, _2));
  } else {
    LOG_INFO("Request send");

    // Start the read deadline timer.
    AsyncWaitDeadlineTimer(read_timeout_);

    // Start to read response.
    AsyncRead();
  }
}

void Client::OnWriteBody(boost::system::error_code ec, std::size_t legnth) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  // Continue to write the next payload of body.
  AsyncWriteBody();
}

void Client::HandleWriteError(boost::system::error_code ec) {
  if (ec == boost::asio::error::operation_aborted) {
    // Socket has been closed by OnDeadlineTimer() or DoClose().
    LOG_WARN("Write operation aborted");
  } else {
    LOG_ERRO("Socket write error (%s)", ec.message().c_str());
    DoClose();
  }

  error_.Set(Error::kSocketWriteError, "Socket write error");
  FinishRequest();
}

void Client::AsyncRead() {
  socket_->AsyncReadSome(std::bind(&Client::OnRead, this, _1, _2), &buffer_);
}

void Client::OnRead(boost::system::error_code ec, std::size_t length) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or DoClose().
      LOG_WARN("Read operation aborted");
    } else {
      LOG_ERRO("Socket read error (%s)", ec.message().c_str());
      DoClose();
    }

    error_.Set(Error::kSocketReadError, "Socket read error");
    FinishRequest();
    return;
  }

  length_read_ += length;

  LOG_INFO("Read length: %u", length);

  // Parse the piece of data just read.
  if (!response_parser_.Parse(buffer_.data(), length)) {
    LOG_ERRO("Failed to parse the response");
    DoClose();
    error_.Set(Error::kParseError, "Response parse error");
    FinishRequest();
    return;
  }

  // Inform progress callback if it's specified.
  if (progress_callback_) {
    if (response_parser_.header_ended()) {
      // NOTE: Need to get rid of the header length.
      progress_callback_(length_read_ - response_parser_.header_length(),
                         response_parser_.content_length());
    }
  }

  if (response_parser_.finished()) {
    LOG_VERB("Response:\n%s", response_->Dump().c_str());

    if (response_->IsConnectionKeepAlive()) {
      LOG_INFO("Keep the socket connection alive");
    } else {
      DoClose();
    }

    // Stop trying to read once all content has been received, because some
    // servers will block extra call to read_some().

    LOG_INFO("Finished to read the response");
    FinishRequest();
    return;
  }

  // Continue to read the response.
  AsyncRead();
}

void Client::AsyncWaitDeadlineTimer(int seconds) {
  if (seconds <= 0) {
    deadline_timer_stopped_ = true;
    return;
  }

  LOG_VERB("Async wait deadline timer");

  deadline_timer_stopped_ = false;

  deadline_timer_.expires_after(std::chrono::seconds(seconds));
  deadline_timer_.async_wait(std::bind(&Client::OnDeadlineTimer, this, _1));
}

void Client::OnDeadlineTimer(boost::system::error_code ec) {
  LOG_VERB("On deadline timer");

  deadline_timer_stopped_ = true;

  // deadline_timer_.cancel() was called.
  if (ec == boost::asio::error::operation_aborted) {
    LOG_VERB("Deadline timer canceled");
    return;
  }

  LOG_WARN("Timeout");

  // Cancel the async operations on the socket.
  // OnXxx() will be called with `error::operation_aborted`.
  if (connected_) {
    DoClose();
  } else {
    socket_->Close();
  }

  error_.set_timeout(true);
}

void Client::StopDeadlineTimer() {
  if (deadline_timer_stopped_) {
    return;
  }

  LOG_INFO("Cancel deadline timer");

  try {
    // Cancel the async wait operation on this timer.
    deadline_timer_.cancel();
  } catch (const boost::system::system_error& e) {
    LOG_ERRO("Deadline timer cancel error: %s", e.what());
  }

  deadline_timer_stopped_ = true;
}

void Client::FinishRequest() {
  request_mutex_.lock();

  if (!request_finished_) {
    request_finished_ = true;

    request_mutex_.unlock();
    request_cv_.notify_one();
  } else {
    request_mutex_.unlock();
  }
}

}  // namespace webcc
