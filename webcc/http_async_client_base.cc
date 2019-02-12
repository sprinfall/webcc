#include "webcc/http_async_client_base.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

HttpAsyncClientBase::HttpAsyncClientBase(boost::asio::io_context& io_context,
                                         std::size_t buffer_size)
    : resolver_(io_context),
      buffer_(buffer_size == 0 ? kBufferSize : buffer_size),
      deadline_(io_context),
      timeout_seconds_(kMaxReadSeconds),
      stopped_(false),
      timed_out_(false) {
}

void HttpAsyncClientBase::SetTimeout(int seconds) {
  if (seconds > 0) {
    timeout_seconds_ = seconds;
  }
}

void HttpAsyncClientBase::Request(std::shared_ptr<HttpRequest> request,
                                  HttpResponseCallback response_callback) {
  assert(request);
  assert(response_callback);

  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));

  stopped_ = false;
  timed_out_ = false;

  LOG_VERB("HTTP request:\n%s", request->Dump(4, "> ").c_str());

  request_ = request;
  response_callback_ = response_callback;

  DoResolve(kHttpSslPort);
}

void HttpAsyncClientBase::Stop() {
  LOG_INFO("The user asks to cancel the request.");

  DoStop();
}

void HttpAsyncClientBase::DoResolve(const std::string& default_port) {
  auto handler = std::bind(&HttpAsyncClientBase::OnResolve, shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);

  resolver_.async_resolve(tcp::v4(), request_->host(),
                          request_->port(default_port), handler);
}

void HttpAsyncClientBase::OnResolve(boost::system::error_code ec,
                                    tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Host resolve error (%s): %s, %s.", ec.message().c_str(),
             request_->host().c_str(), request_->port().c_str());

    response_callback_(response_, kHostResolveError, timed_out_);
  } else {
    LOG_VERB("Host resolved.");

    DoConnect(endpoints);
  }
}

void HttpAsyncClientBase::DoConnect(const Endpoints& endpoints) {
  auto handler = std::bind(&HttpAsyncClientBase::OnConnect,
                           shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);

  SocketAsyncConnect(endpoints, std::move(handler));
}
  
void HttpAsyncClientBase::OnConnect(boost::system::error_code ec,
                                    tcp::endpoint endpoint) {
  if (ec) {
    LOG_ERRO("Socket connect error (%s).", ec.message().c_str());
    DoStop();
    response_callback_(response_, kEndpointConnectError, timed_out_);
    return;
  }

  LOG_VERB("Socket connected.");

  // Even though the connect operation notionally succeeded, the user could
  // have stopped the operation by calling Stop(). And if we started the
  // deadline timer, it could also be stopped due to timeout.
  if (stopped_) {
    // TODO: Use some other error.
    response_callback_(response_, kEndpointConnectError, timed_out_);
    return;
  }

  // Connection established.
  OnConnected();
}

void HttpAsyncClientBase::DoWrite() {
  // NOTE:
  // It doesn't make much sense to set a timeout for socket write.
  // I find that it's almost impossible to simulate a situation in the server
  // side to test this timeout.

  SocketAsyncWrite(std::bind(&HttpAsyncClientBase::OnWrite, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2));
}

void HttpAsyncClientBase::OnWrite(boost::system::error_code ec,
                                  std::size_t /*length*/) {
  if (stopped_) {
    // TODO: Use some other error.
    response_callback_(response_, kSocketWriteError, timed_out_);
    return;
  }

  if (ec) {
    LOG_ERRO("Socket write error (%s).", ec.message().c_str());
    DoStop();
    response_callback_(response_, kSocketWriteError, timed_out_);
  } else {
    LOG_INFO("Request sent.");
    LOG_VERB("Read response (timeout: %ds)...", timeout_seconds_);

    deadline_.expires_from_now(boost::posix_time::seconds(timeout_seconds_));
    DoWaitDeadline();

    DoRead();
  }
}

void HttpAsyncClientBase::DoRead() {
  auto handler = std::bind(&HttpAsyncClientBase::OnRead, shared_from_this(),
                           std::placeholders::_1, std::placeholders::_2);
  SocketAsyncReadSome(std::move(handler));
}

void HttpAsyncClientBase::OnRead(boost::system::error_code ec,
                                 std::size_t length) {
  LOG_VERB("Socket async read handler.");

  if (ec || length == 0) {
    DoStop();
    LOG_ERRO("Socket read error (%s).", ec.message().c_str());
    response_callback_(response_, kSocketReadError, timed_out_);
    return;
  }

  LOG_INFO("Read data, length: %u.", length);

  // Parse the response piece just read.
  // If the content has been fully received, |finished()| will be true.
  if (!response_parser_->Parse(buffer_.data(), length)) {
    DoStop();
    LOG_ERRO("Failed to parse HTTP response.");
    response_callback_(response_, kHttpError, timed_out_);
    return;
  }

  if (response_parser_->finished()) {
    DoStop();

    LOG_INFO("Finished to read and parse HTTP response.");
    LOG_VERB("HTTP response:\n%s", response_->Dump(4, "> ").c_str());

    response_callback_(response_, kNoError, timed_out_);
    return;
  }

  if (!stopped_) {
    DoRead();
  }
}

void HttpAsyncClientBase::DoWaitDeadline() {
  deadline_.async_wait(std::bind(&HttpAsyncClientBase::OnDeadline,
                                 shared_from_this(), std::placeholders::_1));
}

void HttpAsyncClientBase::OnDeadline(boost::system::error_code ec) {
  if (stopped_) {
    return;
  }

  LOG_VERB("OnDeadline.");

  // NOTE: Can't check this:
  //   if (ec == boost::asio::error::operation_aborted) {
  //     LOG_VERB("Deadline timer canceled.");
  //     return;
  //   }

  if (deadline_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    LOG_WARN("HTTP client timed out.");
    timed_out_ = true;
    Stop();
    return;
  }

  // Put the actor back to sleep.
  DoWaitDeadline();
}

void HttpAsyncClientBase::DoStop() {
  if (stopped_) {
    return;
  }

  stopped_ = true;

  LOG_INFO("Close socket...");

  boost::system::error_code ec;
  SocketClose(&ec);

  if (ec) {
    LOG_ERRO("Socket close error (%s).", ec.message().c_str());
  }

  LOG_INFO("Cancel deadline timer...");
  deadline_.cancel();
}

}  // namespace webcc
