#include "webcc/http_client.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"

#if 0
#include "boost/asio.hpp"
#else
#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#endif

#include "webcc/logger.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

// NOTE:
// The timeout control is inspired by the following Asio example:
//     example\cpp03\timeouts\blocking_tcp_client.cpp

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

static const int kConnectMaxSeconds = 10;
static const int kSendMaxSeconds = 10;
static const int kReceiveMaxSeconds = 30;

////////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient()
    : socket_(io_context_)
    , timeout_seconds_(kReceiveMaxSeconds)
    , deadline_timer_(io_context_) {

  deadline_timer_.expires_at(boost::posix_time::pos_infin);

  // Start the persistent actor that checks for deadline expiry.
  CheckDeadline();
}

Error HttpClient::MakeRequest(const HttpRequest& request,
                              HttpResponse* response) {
  assert(response != NULL);

  Error error = kNoError;

  if ((error = Connect(request)) != kNoError) {
    return error;
  }
  
  // Send HTTP request.

  if ((error = SendReqeust(request)) != kNoError) {
    return error;
  }

  // Read and parse HTTP response.

  parser_ = std::make_unique<HttpResponseParser>(response);

  error = ReadResponse(response);

  return error;
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
    LOG_ERRO("cannot resolve host: %s, %s",
             request.host().c_str(),
             port.c_str());

    return kHostResolveError;
  }

  deadline_timer_.expires_from_now(
      boost::posix_time::seconds(kConnectMaxSeconds));

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
    if (ec) {
      return kEndpointConnectError;
    } else {
      return kSocketTimeoutError;
    }
  }

  return kNoError;
}

Error HttpClient::SendReqeust(const HttpRequest& request) {
  LOG_VERB("http request:\n{\n%s}", request.Dump().c_str());

  deadline_timer_.expires_from_now(boost::posix_time::seconds(kSendMaxSeconds));

  boost::system::error_code ec = boost::asio::error::would_block;

  boost::asio::async_write(socket_,
                           request.ToBuffers(),
                           boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    return kSocketWriteError;
  }

  return kNoError;
}

Error HttpClient::ReadResponse(HttpResponse* response) {
  deadline_timer_.expires_from_now(
      boost::posix_time::seconds(timeout_seconds_));

  boost::system::error_code ec = boost::asio::error::would_block;
  Error error = kNoError;

  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, &ec, &error, response](boost::system::error_code inner_ec,
                                    std::size_t length) {
        ec = inner_ec;

        if (inner_ec || length == 0) {
          error = kSocketReadError;
        } else {
          // Parse the response piece just read.
          // If the content has been fully received, next time flag "finished_"
          // will be set.
          error = parser_->Parse(buffer_.data(), length);

          if (error != kNoError) {
            LOG_ERRO("failed to parse http response.");
            return;
          }
          
          if (parser_->finished()) {
            // Stop trying to read once all content has been received,
            // because some servers will block extra call to read_some().
            return;
          }

          ReadResponse(response);
        }
      });

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (error == kNoError) {
    LOG_VERB("http response:\n{\n%s}", response->Dump().c_str());
  }

  return error;
}

void HttpClient::CheckDeadline() {
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
  deadline_timer_.async_wait(std::bind(&HttpClient::CheckDeadline, this));
}

}  // namespace webcc
