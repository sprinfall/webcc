#include "webcc/http_client.h"

#include <string>

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "webcc/logger.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

// NOTE:
// The timeout control is inspired by the following Asio example:
//     example\cpp03\timeouts\blocking_tcp_client.cpp

namespace webcc {

HttpClient::HttpClient()
    : socket_(io_context_),
      timeout_seconds_(kMaxReceiveSeconds),
      deadline_timer_(io_context_) {
  deadline_timer_.expires_at(boost::posix_time::pos_infin);
}

bool HttpClient::Request(const HttpRequest& request) {
  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));

  timeout_occurred_ = false;

  // Start the persistent actor that checks for deadline expiry.
  CheckDeadline();

  if ((error_ = Connect(request)) != kNoError) {
    return false;
  }

  if ((error_ = SendReqeust(request)) != kNoError) {
    return false;
  }

  if ((error_ = ReadResponse()) != kNoError) {
    return false;
  }

  return true;
}

Error HttpClient::Connect(const HttpRequest& request) {
  using boost::asio::ip::tcp;

  tcp::resolver resolver(io_context_);

  std::string port = request.port();
  if (port.empty()) {
    port = "80";
  }

  boost::system::error_code ec;
  auto endpoints = resolver.resolve(tcp::v4(), request.host(), port, ec);

  if (ec) {
    LOG_ERRO("Can't resolve host (%s): %s, %s", ec.message().c_str(),
             request.host().c_str(), port.c_str());
    return kHostResolveError;
  }

  deadline_timer_.expires_from_now(
      boost::posix_time::seconds(kMaxConnectSeconds));

  ec = boost::asio::error::would_block;

  boost::asio::async_connect(socket_,
                             endpoints,
                             boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  // Determine whether a connection was successfully established. The
  // deadline actor may have had a chance to run and close our socket, even
  // though the connect operation notionally succeeded. Therefore we must
  // check whether the socket is still open before deciding if we succeeded
  // or failed.
  if (ec || !socket_.is_open()) {
    if (!ec) {
      timeout_occurred_ = true;
    }
    return kEndpointConnectError;
  }

  return kNoError;
}

Error HttpClient::SendReqeust(const HttpRequest& request) {
  LOG_VERB("HTTP request:\n%s", request.Dump(4, "> ").c_str());

  deadline_timer_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

  boost::system::error_code ec = boost::asio::error::would_block;

  boost::asio::async_write(socket_,
                           request.ToBuffers(),
                           boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  // TODO: timeout_occurred_
  if (ec) {
    return kSocketWriteError;
  }

  return kNoError;
}

Error HttpClient::ReadResponse() {
  Error error = kNoError;
  DoReadResponse(&error);

  if (error == kNoError) {
    LOG_VERB("HTTP response:\n%s", response_->Dump(4, "> ").c_str());
  }

  return error;
}

void HttpClient::DoReadResponse(Error* error) {
  deadline_timer_.expires_from_now(
      boost::posix_time::seconds(timeout_seconds_));

  boost::system::error_code ec = boost::asio::error::would_block;

  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, &ec, error](boost::system::error_code inner_ec,
                         std::size_t length) {
        ec = inner_ec;

        if (inner_ec || length == 0) {
          *error = kSocketReadError;
          LOG_ERRO("Socket read error.");
          return;
        }

        // Parse the response piece just read.
        // If the content has been fully received, next time flag "finished_"
        // will be set.
        if (!response_parser_->Parse(buffer_.data(), length)) {
          *error = kHttpError;
          LOG_ERRO("Failed to parse HTTP response.");
          return;
        }

        if (response_parser_->finished()) {
          // Stop trying to read once all content has been received,
          // because some servers will block extra call to read_some().
          return;
        }

        DoReadResponse(error);
      });

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);
}

void HttpClient::CheckDeadline() {
  if (deadline_timer_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    boost::system::error_code ignored_ec;
    socket_.close(ignored_ec);

    // TODO
    timeout_occurred_ = true;

    deadline_timer_.expires_at(boost::posix_time::pos_infin);
  }

  // Put the actor back to sleep.
  deadline_timer_.async_wait(std::bind(&HttpClient::CheckDeadline, this));
}

}  // namespace webcc
