#include "webcc/http_async_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

namespace webcc {

HttpAsyncClient::HttpAsyncClient(boost::asio::io_context& io_context)
    : socket_(io_context),
      resolver_(new tcp::resolver(io_context)),
      buffer_(kBufferSize),
      deadline_(io_context),
      timeout_seconds_(kMaxReadSeconds),
      stopped_(false),
      timed_out_(false) {
}

void HttpAsyncClient::Request(std::shared_ptr<HttpRequest> request,
                              HttpResponseHandler response_handler) {
  assert(request);
  assert(response_handler);

  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));

  stopped_ = timed_out_ = false;

  LOG_VERB("HTTP request:\n%s", request->Dump(4, "> ").c_str());

  request_ = request;
  response_handler_ = response_handler;

  resolver_->async_resolve(tcp::v4(), request->host(), request->port(kHttpPort),
                           std::bind(&HttpAsyncClient::ResolveHandler,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void HttpAsyncClient::Stop() {
  if (!stopped_) {
    stopped_ = true;

    LOG_INFO("Close socket...");

    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
      LOG_ERRO("Failed to close socket.");
    }

    LOG_INFO("Cancel deadline timer...");
    deadline_.cancel();
  }
}

void HttpAsyncClient::ResolveHandler(boost::system::error_code ec,
                                     tcp::resolver::results_type endpoints) {
  if (ec) {
    LOG_ERRO("Can't resolve host (%s): %s, %s", ec.message().c_str(),
             request_->host().c_str(), request_->port().c_str());
    response_handler_(response_, kHostResolveError, timed_out_);
  } else {
    // Start the connect actor.
    endpoints_ = endpoints;

    // ConnectHandler: void(boost::system::error_code, tcp::endpoint)
    boost::asio::async_connect(socket_, endpoints_,
                               std::bind(&HttpAsyncClient::ConnectHandler,
                                         shared_from_this(),
                                         std::placeholders::_1,
                                         std::placeholders::_2));
  }
}

void HttpAsyncClient::ConnectHandler(boost::system::error_code ec,
                                     tcp::endpoint endpoint) {
  if (ec) {
    LOG_ERRO("Socket connect error (%s).", ec.message().c_str());
    Stop();
    response_handler_(response_, kEndpointConnectError, timed_out_);
    return;
  }

  LOG_VERB("Socket connected.");

  // The deadline actor may have had a chance to run and close our socket, even
  // though the connect operation notionally succeeded.
  if (stopped_) {
    // |timed_out_| should be true in this case.
    LOG_ERRO("Socket connect timed out.");
    response_handler_(response_, kEndpointConnectError, timed_out_);
    return;
  }

  // Connection established.
  AsyncWrite();
}

void HttpAsyncClient::AsyncWrite() {
  if (stopped_) {
    return;
  }

  boost::asio::async_write(socket_,
                           request_->ToBuffers(),
                           std::bind(&HttpAsyncClient::WriteHandler,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void HttpAsyncClient::WriteHandler(boost::system::error_code ec) {
  if (stopped_) {
    return;
  }

  if (ec) {
    Stop();
    response_handler_(response_, kSocketWriteError, timed_out_);
  } else {
    deadline_.expires_from_now(boost::posix_time::seconds(timeout_seconds_));
    AsyncWaitDeadline();

    AsyncRead();
  }
}

void HttpAsyncClient::AsyncRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpAsyncClient::ReadHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpAsyncClient::ReadHandler(boost::system::error_code ec,
                                  std::size_t length) {
  LOG_VERB("Socket async read handler.");

  if (ec || length == 0) {
    Stop();
    LOG_ERRO("Socket read error (%s).", ec.message().c_str());
    response_handler_(response_, kSocketReadError, timed_out_);
    return;
  }

  LOG_INFO("Read data, length: %d.", length);

  bool content_length_parsed = response_parser_->content_length_parsed();

  // Parse the response piece just read.
  // If the content has been fully received, |finished()| will be true.
  if (!response_parser_->Parse(buffer_.data(), length)) {
    Stop();
    LOG_ERRO("Failed to parse HTTP response.");
    response_handler_(response_, kHttpError, timed_out_);
    return;
  }

  if (!content_length_parsed &&
      response_parser_->content_length_parsed()) {
    // Content length just has been parsed.
    AdjustBufferSize(response_parser_->content_length(), &buffer_);
  }

  if (response_parser_->finished()) {
    LOG_INFO("Finished to read and parse HTTP response.");
    LOG_VERB("HTTP response:\n%s", response_->Dump(4, "> ").c_str());
    Stop();
    response_handler_(response_, kNoError, timed_out_);
    return;
  }

  if (!stopped_) {
    AsyncRead();
  }
}

void HttpAsyncClient::AsyncWaitDeadline() {
  deadline_.async_wait(std::bind(&HttpAsyncClient::DeadlineHandler,
                                 shared_from_this(), std::placeholders::_1));
}

void HttpAsyncClient::DeadlineHandler(boost::system::error_code ec) {
  LOG_VERB("Deadline handler.");

  if (ec == boost::asio::error::operation_aborted) {
    LOG_VERB("Deadline timer canceled.");
    return;
  }

  LOG_WARN("HTTP client timed out.");
  timed_out_ = true;

  Stop();
}

}  // namespace webcc
