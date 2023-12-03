#include "webcc/async_client_base.h"

#include <sstream>

#include "boost/asio/connect.hpp"

#include "webcc/internal/globals.h"
#include "webcc/logger.h"

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

AsyncClientBase::AsyncClientBase(boost::asio::io_context& io_context,
                                 std::string_view default_port)
    : io_context_(io_context),
      default_port_(default_port),
      resolver_(io_context),
      deadline_timer_(io_context) {
}

void AsyncClientBase::Close() {
  boost::system::error_code ec;

  if (connected_) {
    connected_ = false;

    LOG_INFO("Shut down and close socket...");

    GetSocket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec) {
      LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
      ec.clear();
    }
  } else {
    LOG_INFO("Close socket...");
    // TODO: resolver_.cancel() ?
  }

  // TODO: Close socket even when it's not connected?
  GetSocket().close(ec);
  if (ec) {
    LOG_WARN("Socket close error (%s)", ec.message().c_str());
  }

  LOG_INFO("Socket closed");

  // End the request if it's ended yet.
  RequestEnd();
}

void AsyncClientBase::AsyncSend(RequestPtr request, bool stream) {
  RequestBegin();

  request_ = request;
  response_.reset(new Response{});
  response_parser_.Init(response_.get(), stream);

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
    AsyncResolve();
  } else {
    AsyncWrite();
  }
}

void AsyncClientBase::AsyncResolve() {
  std::string_view port = request_->port();
  if (port.empty()) {
    port = default_port_;
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
    error_.Set(error_codes::kResolveError, "Host resolve error");
    Close();
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

  boost::asio::async_connect(
      GetSocket(), endpoints,
      std::bind(&AsyncClientBase::OnConnect, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnConnect(boost::system::error_code ec,
                                tcp::endpoint endpoint) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or Close().
      LOG_WARN("Connect operation aborted");
    } else {
      LOG_ERRO("Connect error (%s)", ec.message().c_str());
    }

    error_.Set(error_codes::kConnectError, "Socket connect error");
    Close();
    return;
  }

  LOG_INFO("Socket connected");
  connected_ = true;

  OnConnected();
}

void AsyncClientBase::AsyncWrite() {
  LOG_VERB("Request:\n%s",
           request_->Dump(internal::log_prefix::kOutgoing).c_str());
  LOG_INFO("Send request...");

  AsyncWrite(request_->GetPayload(),
             std::bind(&AsyncClientBase::OnWrite, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnWrite(boost::system::error_code ec,
                              std::size_t /*length*/) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  request_->body()->InitPayload();

  current_length_ = 0;
  total_length_ = request_->body()->GetSize();

  AsyncWriteBody();
}

void AsyncClientBase::AsyncWriteBody() {
  auto payload = request_->body()->NextPayload(true);

  if (!payload.empty()) {
    AsyncWrite(payload, std::bind(&AsyncClientBase::OnWriteBody,
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
                                  std::size_t length) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  current_length_ += length;

  if (progress_callback_ != nullptr) {
    progress_callback_(current_length_, total_length_, false);
  }

  // Continue to write the next payload of body.
  AsyncWriteBody();
}

void AsyncClientBase::HandleWriteError(boost::system::error_code ec) {
  if (ec == boost::asio::error::operation_aborted) {
    // Socket has been closed by OnDeadlineTimer() or Close().
    LOG_WARN("Socket write aborted");
  } else {
    LOG_ERRO("Socket write error (%s)", ec.message().c_str());
  }

  error_.Set(error_codes::kSocketWriteError, "Socket write error");
  Close();
}

void AsyncClientBase::AsyncRead() {
  AsyncReadSome(
      boost::asio::buffer(buffer_),
      std::bind(&AsyncClientBase::OnRead, shared_from_this(), _1, _2));
}

void AsyncClientBase::OnRead(boost::system::error_code ec, std::size_t length) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // Socket has been closed by OnDeadlineTimer() or Close().
      LOG_WARN("Socket read aborted");
    } else {
      LOG_ERRO("Socket read error (%s)", ec.message().c_str());
    }

    error_.Set(error_codes::kSocketReadError, "Socket read error");
    Close();
    return;
  }

  LOG_INFO("Read length: %u", length);

  current_length_ += length;

  // Parse the piece of data just read.
  if (!response_parser_.Parse(buffer_.data(), length)) {
    LOG_ERRO("Response parse error");
    error_.Set(error_codes::kParseError, "Response parse error");
    Close();
    return;
  }

  if (progress_callback_ != nullptr && response_parser_.header_ended()) {
    if (response_parser_.header_just_ended()) {
      current_length_ -= response_parser_.header_length();
      // NOTE: The total length will be kInvalidSize if the content is chunked.
      total_length_ = response_parser_.content_length();
    }

    progress_callback_(current_length_, total_length_, true);
  }

  if (response_parser_.finished()) {
    LOG_VERB("Response:\n%s",
             response_->Dump(internal::log_prefix::kIncoming).c_str());

    if (response_->IsConnectionKeepAlive()) {
      LOG_INFO("Keep the socket connection alive");
    } else {
      //Close();
    }

    // Stop trying to read once all content has been received, because some
    // servers will block extra call to read_some().

    LOG_INFO("Finished to read the response");
    Close();
    return;
  }

  // Continue to read the response.
  AsyncWaitDeadlineTimer(subsequent_read_timeout_);
  AsyncRead();
}

void AsyncClientBase::AsyncWaitDeadlineTimer(int seconds) {
  if (seconds <= 0) {
    deadline_timer_stopped_ = true;
    return;
  }

  LOG_INFO("Async wait deadline timer (%ds)", seconds);

  deadline_timer_stopped_ = false;

  deadline_timer_.expires_after(std::chrono::seconds(seconds));
  deadline_timer_.async_wait(
      std::bind(&AsyncClientBase::OnDeadlineTimer, shared_from_this(), _1));
}

void AsyncClientBase::OnDeadlineTimer(boost::system::error_code ec) {
  LOG_INFO("On deadline timer");

  // NOT HERE!
  //   deadline_timer_stopped_ = true;

  if (ec == boost::asio::error::operation_aborted) {
    // deadline_timer_.cancel() was called.
    // But, a new async-wait on this timer might have already been triggered.
    // So, don't set `deadline_timer_stopped_` to true right now, it's too late.
    // The newly triggered async-wait needs it to be false.
    LOG_INFO("Deadline timer canceled");
    return;
  }

  LOG_WARN("Operation timeout");

  // NOTE:
  // Don't set this flag to true on `error::operation_aborted`.
  // StopDeadlineTimer() sets it.
  deadline_timer_stopped_ = true;

  // Cancel the async operations on the socket by closing it.
  // OnXxx() will be called with `error::operation_aborted`.
  Close();

  error_.set_timeout(true);
}

void AsyncClientBase::StopDeadlineTimer() {
  // This flag is tricky.
  // Don't set it to true when OnDeadlineTimer(error::operation_aborted).
  if (deadline_timer_stopped_) {
    LOG_INFO("Deadline timer already stopped");
    return;
  }

  LOG_INFO("Cancel deadline timer");

  try {
    // Cancel the async wait operation on this timer.
    deadline_timer_.cancel();

  } catch (const boost::system::system_error& e) {
    LOG_ERRO("Deadline timer cancel error: %s", e.what());
  }

  // To check the time needed by canceling a deadline timer.
  LOG_VERB("Cancel deadline timer end");

  deadline_timer_stopped_ = true;
}

}  // namespace webcc
