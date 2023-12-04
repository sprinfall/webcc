#include "webcc/client_base.h"

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

// Cancel the pending asynchronous operations (including async_connect) on the
// socket.
void SocketCancel(SocketType& socket) {
  boost::system::error_code ec;
  socket.cancel(ec);
  if (ec) {
    LOG_WARN("Socket cancel error (%s)", ec.message().c_str());
  }
}

void SocketShutdown(SocketType& socket) {
  LOG_INFO("Shut down socket...");

  boost::system::error_code ec;
  socket.shutdown(tcp::socket::shutdown_both, ec);
  if (ec) {
    LOG_WARN("Socket shutdown error (%s)", ec.message().c_str());
    return;
  }

  LOG_INFO("Socket shut down");
}

void SocketClose(SocketType& socket) {
  LOG_INFO("Close socket...");

  boost::system::error_code ec;
  socket.close(ec);
  if (ec) {
    LOG_WARN("Socket close error (%s)", ec.message().c_str());
    return;
  }

  LOG_INFO("Socket closed");
}

// -----------------------------------------------------------------------------

ClientBase::ClientBase(boost::asio::io_context& io_context,
                       std::string_view default_port)
    : io_context_(io_context),
      default_port_(default_port),
      resolver_(io_context),
      deadline_timer_(io_context) {
}

bool ClientBase::Close() {
  SocketType& socket = GetSocket();

  if (!socket.is_open()) {
    SocketCancel(socket);

    if (connected_) {
      connected_ = false;
      SocketShutdown(socket);
    }

    SocketClose(socket);

  } else {
    // TODO: resolver_.cancel() ?
  }

  return false;
}

void ClientBase::Send(RequestPtr request, bool stream) {
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

void ClientBase::AsyncResolve() {
  std::string_view port = request_->port();
  if (port.empty()) {
    port = default_port_;
  }

  LOG_INFO("Resolve host... (%s)", request_->host().c_str());

  // The protocol depends on the `host`, both V4 and V6 are supported.
  resolver_.async_resolve(
      request_->host(), port,
      std::bind(&ClientBase::OnResolve, shared_from_this(), _1, _2));
}

void ClientBase::OnResolve(boost::system::error_code ec,
                           tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Host resolve error (%s)", ec.message().c_str());
    error_.Set(error_codes::kResolveError, "Host resolve error");
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

  // GetSocket().is_open() -> false

  boost::asio::async_connect(
      GetSocket(), endpoints,
      std::bind(&ClientBase::OnConnect, shared_from_this(), _1, _2));

  // GetSocket().is_open() -> true
}

void ClientBase::OnConnect(boost::system::error_code ec,
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
    Close();  // Socket is open now, close it.
    return;
  }

  LOG_INFO("Socket connected");
  connected_ = true;

  OnConnected();
}

void ClientBase::AsyncWrite() {
  LOG_VERB("Request:\n%s",
           request_->Dump(internal::log_prefix::kOutgoing).c_str());
  LOG_INFO("Send request...");

  AsyncWrite(request_->GetPayload(),
             std::bind(&ClientBase::OnWrite, shared_from_this(), _1, _2));
}

void ClientBase::OnWrite(boost::system::error_code ec, std::size_t /*length*/) {
  if (ec) {
    HandleWriteError(ec);
    return;
  }

  request_->body()->InitPayload();

  current_length_ = 0;
  total_length_ = request_->body()->GetSize();

  AsyncWriteBody();
}

void ClientBase::AsyncWriteBody() {
  auto payload = request_->body()->NextPayload(true);

  if (!payload.empty()) {
    AsyncWrite(payload,
               std::bind(&ClientBase::OnWriteBody, shared_from_this(), _1, _2));
  } else {
    LOG_INFO("Request sent");

    // Start the read deadline timer.
    AsyncWaitDeadlineTimer(read_timeout_);

    // Start to read response.
    AsyncRead();
  }
}

void ClientBase::OnWriteBody(boost::system::error_code ec, std::size_t length) {
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

void ClientBase::HandleWriteError(boost::system::error_code ec) {
  if (ec == boost::asio::error::operation_aborted) {
    LOG_WARN("Socket write aborted");
  } else {
    LOG_ERRO("Socket write error (%s)", ec.message().c_str());
  }

  error_.Set(error_codes::kSocketWriteError, "Socket write error");
  Close();
}

void ClientBase::AsyncRead() {
  AsyncReadSome(boost::asio::buffer(buffer_),
                std::bind(&ClientBase::OnRead, shared_from_this(), _1, _2));
}

void ClientBase::OnRead(boost::system::error_code ec, std::size_t length) {
  StopDeadlineTimer();

  if (ec) {
    if (ec == boost::asio::error::operation_aborted) {
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

    if (!response_->IsConnectionKeepAlive()) {
      Close();
    } else {
      LOG_INFO("Keep the socket connection alive");
    }

    // Stop trying to read once all content has been received, because some
    // servers will block extra call to read_some().

    LOG_INFO("Finished to read the response");
    return;
  }

  // Continue to read the response.
  AsyncWaitDeadlineTimer(subsequent_read_timeout_);
  AsyncRead();
}

void ClientBase::AsyncWaitDeadlineTimer(int seconds) {
  if (seconds <= 0) {
    deadline_timer_active_ = false;
    return;
  }

  LOG_INFO("Async wait deadline timer (%ds)", seconds);

  deadline_timer_active_ = true;

  deadline_timer_.expires_after(std::chrono::seconds(seconds));
  deadline_timer_.async_wait(
      std::bind(&ClientBase::OnDeadlineTimer, shared_from_this(), _1));
}

void ClientBase::OnDeadlineTimer(boost::system::error_code ec) {
  LOG_INFO("On deadline timer");

  // NOT HERE!
  //   deadline_timer_active_ = false;

  if (ec == boost::asio::error::operation_aborted) {
    // deadline_timer_.cancel() was called.
    // But, a new async-wait on this timer might have already been triggered.
    // So, don't set `deadline_timer_active_` to false right now, it's too late.
    // The newly triggered async-wait needs it to be false.
    LOG_INFO("Deadline timer canceled");
    return;
  }

  LOG_WARN("Operation timeout");

  // NOTE:
  // Don't set this flag to false on `error::operation_aborted`.
  // StopDeadlineTimer() sets it.
  deadline_timer_active_ = false;

  // Cancel the asynchronous operations on the socket.
  // OnXxx() will be called with `error::operation_aborted` and it will call
  // Close() to shut down and close the socket properly.
  GetSocket().cancel(ec);

  error_.set_timeout(true);
}

void ClientBase::StopDeadlineTimer() {
  if (!deadline_timer_active_) {
    return;
  }

  LOG_INFO("Cancel deadline timer");

  try {
    // Cancel the asynchronous wait operation on this timer.
    deadline_timer_.cancel();

  } catch (const boost::system::system_error& e) {
    LOG_ERRO("Deadline timer cancel error: %s", e.what());
  }

  // To check the time needed by canceling a deadline timer.
  LOG_VERB("Cancel deadline timer end");

  deadline_timer_active_ = false;
}

}  // namespace webcc
