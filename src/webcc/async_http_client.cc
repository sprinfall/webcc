#include "webcc/async_http_client.h"

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/utility.h"

// NOTE:
// The timeout control is inspired by the following Asio example:
//     example\cpp03\timeouts\async_tcp_client.cpp

namespace webcc {

AsyncHttpClient::AsyncHttpClient(boost::asio::io_context& io_context)
    : socket_(io_context),
      timeout_seconds_(kMaxReceiveSeconds),
      deadline_(io_context) {
  resolver_.reset(new tcp::resolver(io_context));
  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));
}

Error AsyncHttpClient::Request(std::shared_ptr<HttpRequest> request,
                               HttpResponseHandler response_handler) {
  assert(request);
  assert(response_handler);

  LOG_VERB("HTTP request:\n%s", request->Dump(4, "> ").c_str());

  request_ = request;
  response_handler_ = response_handler;

  std::string port = request->port();
  if (port.empty()) {
    port = "80";
  }

  auto handler = std::bind(&AsyncHttpClient::ResolveHandler,
                           shared_from_this(),
                           std::placeholders::_1,
                           std::placeholders::_2);

  resolver_->async_resolve(tcp::v4(), request->host(), port, handler);

  return kNoError;
}

void AsyncHttpClient::Stop() {
  stopped_ = true;
  boost::system::error_code ignored_ec;
  socket_.close(ignored_ec);
  deadline_.cancel();
}

void AsyncHttpClient::ResolveHandler(boost::system::error_code ec,
                                     tcp::resolver::results_type results) {
  if (ec) {
    LOG_ERRO("Can't resolve host (%s): %s, %s", ec.message().c_str(),
             request_->host().c_str(), request_->port().c_str());
    response_handler_(response_, kHostResolveError, timed_out_);
  } else {
    // Start the connect actor.
    endpoints_ = results;
    AsyncConnect(endpoints_.begin());

    // Start the deadline actor. You will note that we're not setting any
    // particular deadline here. Instead, the connect and input actors will
    // update the deadline prior to each asynchronous operation.
    deadline_.async_wait(std::bind(&AsyncHttpClient::CheckDeadline, this));
  }
}

void AsyncHttpClient::AsyncConnect(EndpointIterator endpoint_iter) {
  if (endpoint_iter != endpoints_.end()) {
    LOG_VERB("Connecting to [%s]...",
             EndpointToString(endpoint_iter->endpoint()).c_str());

    // Set a deadline for the connect operation.
    deadline_.expires_from_now(boost::posix_time::seconds(kMaxConnectSeconds));

    timed_out_ = false;

    // Start the asynchronous connect operation.
    socket_.async_connect(endpoint_iter->endpoint(),
                          std::bind(&AsyncHttpClient::ConnectHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    endpoint_iter));
  } else {
    // There are no more endpoints to try. Shut down the client.
    Stop();
    response_handler_(response_, kEndpointConnectError, timed_out_);
  }
}

void AsyncHttpClient::ConnectHandler(boost::system::error_code ec,
                                     EndpointIterator endpoint_iter) {
  if (stopped_) {
    return;
  }

  if (!socket_.is_open()) {
    // The async_connect() function automatically opens the socket at the start
    // of the asynchronous operation. If the socket is closed at this time then
    // the timeout handler must have run first.
    LOG_WARN("Connect timed out.");

    // Try the next available endpoint.
    AsyncConnect(++endpoint_iter);
  } else if (ec) {    
    // The connect operation failed before the deadline expired.
    // We need to close the socket used in the previous connection attempt
    // before starting a new one.
    socket_.close();

    // Try the next available endpoint.
    AsyncConnect(++endpoint_iter);
  } else {
    // Connection established.
    AsyncWrite();
  }
}

void AsyncHttpClient::AsyncWrite() {
  if (stopped_) {
    return;
  }

  deadline_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

  boost::asio::async_write(socket_,
                           request_->ToBuffers(),
                           std::bind(&AsyncHttpClient::WriteHandler,
                                     shared_from_this(),
                                     std::placeholders::_1));
}

void AsyncHttpClient::WriteHandler(boost::system::error_code ec) {
  if (stopped_) {
    return;
  }

  if (ec) {
    Stop();
    response_handler_(response_, kSocketWriteError, timed_out_);
  } else {
    deadline_.expires_from_now(boost::posix_time::seconds(timeout_seconds_));
    AsyncRead();
  }
}

void AsyncHttpClient::AsyncRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&AsyncHttpClient::ReadHandler,
                                    shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void AsyncHttpClient::ReadHandler(boost::system::error_code ec,
                                  std::size_t length) {
  if (stopped_) {
    return;
  }

  if (ec || length == 0) {
    Stop();
    response_handler_(response_, kSocketReadError, timed_out_);
    return;
  }

  // Parse the response piece just read.
  // If the content has been fully received, |finished()| will be true.
  if (!response_parser_->Parse(buffer_.data(), length)) {
    Stop();
    response_handler_(response_, kHttpError, timed_out_);
    return;
  }

  if (response_parser_->finished()) {
    LOG_VERB("HTTP response:\n%s", response_->Dump(4, "> ").c_str());
    Stop();
    response_handler_(response_, kNoError, timed_out_);
    return;
  }

  AsyncRead();
}

void AsyncHttpClient::CheckDeadline() {
  if (stopped_) {
    return;
  }

  if (deadline_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    boost::system::error_code ignored_ec;
    socket_.close(ignored_ec);

    deadline_.expires_at(boost::posix_time::pos_infin);

    timed_out_ = true;
  }

  // Put the actor back to sleep.
  deadline_.async_wait(std::bind(&AsyncHttpClient::CheckDeadline,
                                 shared_from_this()));
}

}  // namespace webcc
