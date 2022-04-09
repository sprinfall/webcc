#include "webcc/async_client_base.h"

#include <sstream>

#include "webcc/logger.h"
#include "webcc/socket.h"

using boost::asio::ip::tcp;
using namespace std::placeholders;

namespace webcc {

// -----------------------------------------------------------------------------

// TCP endpoint to string.
static std::string EndpointToString(const tcp::endpoint& endpoint) {
  std::ostringstream ss;
  ss << endpoint;
  if (endpoint.protocol() == tcp::v4()) {
    ss << ", v4";
  } else if (endpoint.protocol() == tcp::v6()) {
    ss << ", v6";
  }
  return ss.str();
}

// -----------------------------------------------------------------------------

AsyncClientBase::AsyncClientBase(boost::asio::io_context& io_context)
    : io_context_(io_context),
      resolver_(io_context),
      deadline_timer_(io_context) {
}

void AsyncClientBase::Close() {
  CloseSocket();

  // Don't call FinishRequest() from here! It will be called in the handler
  // OnXxx with `error::operation_aborted`.
}

void AsyncClientBase::AsyncSend(RequestPtr request, bool stream) {
  error_ = Error{};

  request_ = request;

  response_.reset(new Response{});
  response_parser_.Init(response_.get(), stream);

  length_read_ = 0;

  if (buffer_.size() != buffer_size_) {
    LOG_INFO("Resize buffer: %u -> %u", buffer_.size(), buffer_size_);
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
    CreateSocket();
    Resolve();
  } else {
    AsyncWrite();
  }
}

void AsyncClientBase::CloseSocket() {
  if (connected_) {
    connected_ = false;
    if (socket_) {
      LOG_INFO("Shutdown and close socket...");
      socket_->Shutdown();
      socket_->Close();
      LOG_INFO("Socket closed");
    }
  } else {
    // TODO: resolver_.cancel() ?
    if (socket_) {
      LOG_INFO("Close socket...");
      socket_->Close();
      LOG_INFO("Socket closed");
    }
  }
}

void AsyncClientBase::AsyncResolve(std::string_view default_port) {
  std::string port = request_->port();
  if (port.empty()) {
    port = default_port;
  }

  LOG_INFO("Resolve host... (%s)", request_->host().c_str());

  // The protocol depends on the `host`, both V4 and V6 are supported.
  resolver_.async_resolve(
      request_->host(), port,
      std::bind(&AsyncClientBase::OnResolve, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnResolve(boost::system::error_code ec,
                                tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Host resolve error (%s)", ec.message().c_str());
    error_.Set(Error::kResolveError, "Host resolve error");
    RequestEnd();
    return;
  }

  LOG_INFO("Host resolved");

#if 0  // for debug
  LOG_USER("Endpoints resolved:");
  for (auto& endpoint : endpoints) {
    LOG_USER("\t- %s", EndpointToString(endpoint).c_str());
  }
#endif

  AsyncWaitDeadlineTimer(connect_timeout_);

  LOG_INFO("Connect socket...");

  socket_->AsyncConnect(
      request_->host(), endpoints,
      std::bind(&AsyncClientBase::OnConnect, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnConnect(boost::system::error_code ec,
                                tcp::endpoint endpoint) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or CloseSocket().
      LOG_WARN("Connect operation aborted");
    } else {
      LOG_ERRO("Connect error (%s)", ec.message().c_str());
      // No need to close socket since no async operation is on it.
      //   socket_->Close();
    }

    error_.Set(Error::kConnectError, "Socket connect error");
    RequestEnd();
    return;
  }

  LOG_INFO("Socket connected");

  connected_ = true;

  AsyncWrite();
}

void AsyncClientBase::AsyncWrite() {
  LOG_VERB("Request:\n%s", request_->Dump(log_prefix::kOutgoing).c_str());
  LOG_INFO("Send request...");

  socket_->AsyncWrite(
      request_->GetPayload(),
      std::bind(&AsyncClientBase::OnWrite, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnWrite(boost::system::error_code ec,
                              std::size_t length) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  request_->body()->InitPayload();

  AsyncWriteBody();
}

void AsyncClientBase::AsyncWriteBody() {
  auto p = request_->body()->NextPayload(true);

  if (!p.empty()) {
    socket_->AsyncWrite(p, std::bind(&AsyncClientBase::OnWriteBody,
                                     shared_from_this(), _1, _2));
  } else {
    LOG_INFO("Request sent");

    // Start the read deadline timer.
    AsyncWaitDeadlineTimer(read_timeout_);

    // Start to read response.
    AsyncRead();
  }
}

void AsyncClientBase::OnWriteBody(boost::system::error_code ec,
                                  std::size_t legnth) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  // Continue to write the next payload of body.
  AsyncWriteBody();
}

void AsyncClientBase::HandleWriteError(boost::system::error_code ec) {
  if (ec == boost::asio::error::operation_aborted) {
    // Socket has been closed by OnDeadlineTimer() or CloseSocket().
    LOG_WARN("Socket write aborted");
  } else {
    LOG_ERRO("Socket write error (%s)", ec.message().c_str());
    CloseSocket();
  }

  error_.Set(Error::kSocketWriteError, "Socket write error");
  RequestEnd();
}

void AsyncClientBase::AsyncRead() {
  socket_->AsyncReadSome(
      std::bind(&AsyncClientBase::OnRead, shared_from_this(), _1, _2),
      &buffer_);
}

void AsyncClientBase::OnRead(boost::system::error_code ec, std::size_t length) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or CloseSocket().
      LOG_WARN("Socket read aborted");
    } else {
      LOG_ERRO("Socket read error (%s)", ec.message().c_str());
      CloseSocket();
    }

    error_.Set(Error::kSocketReadError, "Socket read error");
    RequestEnd();
    return;
  }

  length_read_ += length;

  LOG_INFO("Read length: %u", length);

  // Parse the piece of data just read.
  if (!response_parser_.Parse(buffer_.data(), length)) {
    LOG_ERRO("Response parse error");
    CloseSocket();
    error_.Set(Error::kParseError, "Response parse error");
    RequestEnd();
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
    LOG_VERB("Response:\n%s", response_->Dump(log_prefix::kIncoming).c_str());

    if (response_->IsConnectionKeepAlive()) {
      LOG_INFO("Keep the socket connection alive");
    } else {
      CloseSocket();
    }

    // Stop trying to read once all content has been received, because some
    // servers will block extra call to read_some().

    LOG_INFO("Finished to read the response");
    RequestEnd();
    return;
  }

  // Continue to read the response.
  AsyncRead();
}

void AsyncClientBase::AsyncWaitDeadlineTimer(int seconds) {
  if (seconds <= 0) {
    deadline_timer_stopped_ = true;
    return;
  }

  LOG_INFO("Async wait deadline timer");

  deadline_timer_stopped_ = false;

  deadline_timer_.expires_after(std::chrono::seconds(seconds));
  deadline_timer_.async_wait(
      std::bind(&AsyncClientBase::OnDeadlineTimer, shared_from_this(), _1));
}

void AsyncClientBase::OnDeadlineTimer(boost::system::error_code ec) {
  LOG_INFO("On deadline timer");

  deadline_timer_stopped_ = true;

  // deadline_timer_.cancel() was called.
  if (ec == boost::asio::error::operation_aborted) {
    LOG_INFO("Deadline timer canceled");
    return;
  }

  LOG_WARN("Operation timeout");

  // Cancel the async operations on the socket by closing it.
  // OnXxx() will be called with `error::operation_aborted`.
  CloseSocket();

  error_.set_timeout(true);
}

void AsyncClientBase::StopDeadlineTimer() {
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

}  // namespace webcc
