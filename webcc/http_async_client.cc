#include "webcc/http_async_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

HttpAsyncClient::HttpAsyncClient(boost::asio::io_context& io_context,
                                 std::size_t buffer_size)
    : socket_(io_context),
      resolver_(io_context),
      buffer_(buffer_size == 0 ? kBufferSize : buffer_size),
      deadline_(io_context),
      timeout_seconds_(kMaxReadSeconds),
      stopped_(false),
      timed_out_(false) {
}

void HttpAsyncClient::SetTimeout(int seconds) {
  if (seconds > 0) {
    timeout_seconds_ = seconds;
  }
}

void HttpAsyncClient::Request(std::shared_ptr<HttpRequest> request,
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

  resolver_.async_resolve(tcp::v4(), request->host(), request->port(kHttpPort),
                          std::bind(&HttpAsyncClient::OnResolve,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpAsyncClient::Stop() {
  LOG_INFO("The user asks to cancel the request.");

  DoStop();
}

void HttpAsyncClient::OnResolve(boost::system::error_code ec,
                                tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Host resolve error (%s): %s, %s.", ec.message().c_str(),
             request_->host().c_str(), request_->port().c_str());
    response_callback_(response_, kHostResolveError, timed_out_);
  } else {
    LOG_VERB("Connect to server...");

    // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
    boost::asio::async_connect(socket_, endpoints,
                               std::bind(&HttpAsyncClient::OnConnect,
                                         shared_from_this(),
                                         std::placeholders::_1,
                                         std::placeholders::_2));
  }
}

void HttpAsyncClient::OnConnect(boost::system::error_code ec,
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
  DoWrite();
}

void HttpAsyncClient::DoWrite() {
  // NOTE:
  // It doesn't make much sense to set a timeout for socket write.
  // I find that it's almost impossible to simulate a situation in the server
  // side to test this timeout.

  boost::asio::async_write(socket_,
                           request_->ToBuffers(),
                           std::bind(&HttpAsyncClient::OnWrite,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void HttpAsyncClient::OnWrite(boost::system::error_code ec) {
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

void HttpAsyncClient::DoRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpAsyncClient::OnRead,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpAsyncClient::OnRead(boost::system::error_code ec,
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

void HttpAsyncClient::DoWaitDeadline() {
  deadline_.async_wait(std::bind(&HttpAsyncClient::OnDeadline,
                                 shared_from_this(), std::placeholders::_1));
}

void HttpAsyncClient::OnDeadline(boost::system::error_code ec) {
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

void HttpAsyncClient::DoStop() {
  if (stopped_) {
    return;
  }

  stopped_ = true;

  LOG_INFO("Close socket...");

  boost::system::error_code ec;
  socket_.close(ec);
  if (ec) {
    LOG_ERRO("Socket close error (%s).", ec.message().c_str());
  }

  LOG_INFO("Cancel deadline timer...");
  deadline_.cancel();
}

}  // namespace webcc
