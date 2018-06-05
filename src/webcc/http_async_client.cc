#include "webcc/http_async_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"

namespace webcc {

HttpAsyncClient::HttpAsyncClient(boost::asio::io_context& io_context)
    : socket_(io_context),
      timeout_seconds_(kMaxReceiveSeconds),
      deadline_timer_(io_context) {
  resolver_.reset(new tcp::resolver(io_context));
  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));

  deadline_timer_.expires_at(boost::posix_time::pos_infin);

  // Start the persistent actor that checks for deadline expiry.
  CheckDeadline();
}

Error HttpAsyncClient::Request(std::shared_ptr<HttpRequest> request,
                               HttpResponseHandler response_handler) {
  assert(request);
  assert(response_handler);

  request_ = request;
  response_handler_ = response_handler;

  std::string port = request->port();
  if (port.empty()) {
    port = "80";
  }

  auto handler = std::bind(&HttpAsyncClient::ResolveHandler,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);

  resolver_->async_resolve(tcp::v4(), request->host(), port, handler);

  return kNoError;
}

void HttpAsyncClient::ResolveHandler(boost::system::error_code ec,
                                     tcp::resolver::results_type results) {
  if (ec) {
    LOG_ERRO("Can't resolve host (%s): %s, %s", ec.message().c_str(),
             request_->host().c_str(), request_->port().c_str());
    response_handler_(response_, kHostResolveError);
  } else {
    endpoints_ = results;
    AsyncConnect(endpoints_.begin());
  }
}

void HttpAsyncClient::AsyncConnect(tcp::resolver::results_type::iterator endpoint_it) {
  if (endpoint_it != endpoints_.end()) {
    deadline_timer_.expires_from_now(
        boost::posix_time::seconds(kMaxConnectSeconds));

    socket_.async_connect(endpoint_it->endpoint(),
                          std::bind(&HttpAsyncClient::ConnectHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    endpoint_it));
  }
}

void HttpAsyncClient::ConnectHandler(
    boost::system::error_code ec,
    tcp::resolver::results_type::iterator endpoint_it) {
  if (ec) {
    // Will be here if the endpoint is IPv6.
    response_handler_(response_, kEndpointConnectError);
    socket_.close();
    // Try the next available endpoint.
    AsyncConnect(++endpoint_it);
  } else {
    AsyncWrite();
  }
}

void HttpAsyncClient::AsyncWrite() {
  deadline_timer_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

  boost::asio::async_write(socket_,
                           request_->ToBuffers(),
                           std::bind(&HttpAsyncClient::WriteHandler,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void HttpAsyncClient::WriteHandler(boost::system::error_code ec) {
  if (ec) {
    response_handler_(response_, kSocketWriteError);
  } else {
    AsyncRead();
  }
}

void HttpAsyncClient::AsyncRead() {
  deadline_timer_.expires_from_now(
      boost::posix_time::seconds(timeout_seconds_));

  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpAsyncClient::ReadHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void HttpAsyncClient::ReadHandler(boost::system::error_code ec,
                                  std::size_t length) {
  if (ec || length == 0) {
    response_handler_(response_, kSocketReadError);
    return;
  }

  // Parse the response piece just read.
  // If the content has been fully received, |finished()| will be true.
  if (!response_parser_->Parse(buffer_.data(), length)) {
    response_handler_(response_, kHttpError);
    return;
  }

  if (response_parser_->finished()) {
    response_handler_(response_, kHttpError);
    return;
  }

  AsyncRead();
}

void HttpAsyncClient::CheckDeadline() {
  if (deadline_timer_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    boost::system::error_code ignored_ec;
    socket_.close(ignored_ec);

    deadline_timer_.expires_at(boost::posix_time::pos_infin);
  }

  // Put the actor back to sleep.
  deadline_timer_.async_wait(std::bind(&HttpAsyncClient::CheckDeadline,
                                       shared_from_this()));
}

}  // namespace webcc
