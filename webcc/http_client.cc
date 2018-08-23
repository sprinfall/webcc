#include "webcc/http_client.h"

#include <string>

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "webcc/logger.h"

namespace webcc {

// Adjust buffer size according to content length.
// This is to avoid reading too many times.
// Also used by AsyncHttpClient.
void AdjustBufferSize(std::size_t content_length, std::vector<char>* buffer) {
  const std::size_t kMaxTimes = 10;

  // According to test, a client never read more than 200000 bytes a time.
  // So it doesn't make sense to set any larger size, e.g., 1MB.
  const std::size_t kMaxBufferSize = 200000;

  LOG_INFO("Adjust buffer size according to content length.");

  std::size_t min_buffer_size = content_length / kMaxTimes;
  if (min_buffer_size > buffer->size()) {
    buffer->resize(std::min(min_buffer_size, kMaxBufferSize));
    LOG_INFO("Resize read buffer to %u.", buffer->size());
  } else {
    LOG_INFO("Keep the current buffer size: %u.", buffer->size());
  }
}

HttpClient::HttpClient()
    : socket_(io_context_),
      buffer_(kBufferSize),
      deadline_(io_context_),
      timeout_seconds_(kMaxReceiveSeconds),
      stopped_(false),
      timed_out_(false),
      error_(kNoError) {
}

bool HttpClient::Request(const HttpRequest& request) {
  response_.reset(new HttpResponse());
  response_parser_.reset(new HttpResponseParser(response_.get()));

  stopped_ = false;
  timed_out_ = false;

  // Start the persistent actor that checks for deadline expiry.
  deadline_.expires_at(boost::posix_time::pos_infin);
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

  std::string port = request.port(kHttpPort);

  boost::system::error_code ec;
  auto endpoints = resolver.resolve(tcp::v4(), request.host(), port, ec);

  if (ec) {
    LOG_ERRO("Can't resolve host (%s): %s, %s", ec.message().c_str(),
             request.host().c_str(), port.c_str());
    return kHostResolveError;
  }

  LOG_VERB("Connect to server...");

  deadline_.expires_from_now(boost::posix_time::seconds(kMaxConnectSeconds));

  ec = boost::asio::error::would_block;

  // ConnectHandler: void (boost::system::error_code, tcp::endpoint)
  // Using |boost::lambda::var()| is identical to:
  //   boost::asio::async_connect(
  //       socket_, endpoints,
  //       [this, &ec](boost::system::error_code inner_ec, tcp::endpoint) {
  //         ec = inner_ec;
  //       });
  boost::asio::async_connect(socket_, endpoints,
                             boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  // Determine whether a connection was successfully established.
  if (ec) {
    LOG_ERRO("Socket connect error: %s", ec.message().c_str());
    Stop();
    return kEndpointConnectError;
  }

  LOG_VERB("Socket connected.");

  // The deadline actor may have had a chance to run and close our socket, even
  // though the connect operation notionally succeeded.
  if (stopped_) {
    // |timed_out_| should be true in this case.
    LOG_ERRO("Socket connect timed out.");
    return kEndpointConnectError;
  }

  return kNoError;
}

Error HttpClient::SendReqeust(const HttpRequest& request) {
  LOG_VERB("Send request (timeout: %ds)...", kMaxSendSeconds);
  LOG_VERB("HTTP request:\n%s", request.Dump(4, "> ").c_str());

  // NOTE:
  // It doesn't make much sense to set a timeout for socket write.
  // I find that it's almost impossible to simulate a situation in the server
  // side to test this timeout.
  deadline_.expires_from_now(boost::posix_time::seconds(kMaxSendSeconds));

  boost::system::error_code ec = boost::asio::error::would_block;

  // WriteHandler: void (boost::system::error_code, std::size_t)
  boost::asio::async_write(socket_, request.ToBuffers(),
                           boost::lambda::var(ec) = boost::lambda::_1);

  // Block until the asynchronous operation has completed.
  do {
    io_context_.run_one();
  } while (ec == boost::asio::error::would_block);

  if (ec) {
    LOG_ERRO("Socket write error: %s", ec.message().c_str());
    Stop();
    return kSocketWriteError;
  }

  if (stopped_) {
    // |timed_out_| should be true in this case.
    LOG_ERRO("Socket write timed out.");
    return kSocketWriteError;
  }

  return kNoError;
}

Error HttpClient::ReadResponse() {
  LOG_VERB("Read response (timeout: %ds)...", timeout_seconds_);

  deadline_.expires_from_now(boost::posix_time::seconds(timeout_seconds_));

  Error error = kNoError;
  DoReadResponse(&error);

  if (error == kNoError) {
    LOG_VERB("HTTP response:\n%s", response_->Dump(4, "> ").c_str());
  }

  return error;
}

void HttpClient::DoReadResponse(Error* error) {
  boost::system::error_code ec = boost::asio::error::would_block;

  // ReadHandler: void(boost::system::error_code, std::size_t)
  socket_.async_read_some(
      boost::asio::buffer(buffer_),
      [this, &ec, error](boost::system::error_code inner_ec,
                         std::size_t length) {
        ec = inner_ec;

        LOG_VERB("Socket async read handler.");

        if (stopped_) {
          return;
        }

        if (inner_ec || length == 0) {
          Stop();
          *error = kSocketReadError;
          LOG_ERRO("Socket read error.");
          return;
        }

        LOG_INFO("Read data, length: %u.", length);

        bool content_length_parsed = response_parser_->content_length_parsed();

        // Parse the response piece just read.
        if (!response_parser_->Parse(buffer_.data(), length)) {
          Stop();
          *error = kHttpError;
          LOG_ERRO("Failed to parse HTTP response.");
          return;
        }

        if (!content_length_parsed &&
            response_parser_->content_length_parsed()) {
          // Content length just has been parsed.
          AdjustBufferSize(response_parser_->content_length(), &buffer_);
        }

        if (response_parser_->finished()) {
          // Stop trying to read once all content has been received,
          // because some servers will block extra call to read_some().
          Stop();
          LOG_INFO("Finished to read and parse HTTP response.");
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
  if (stopped_) {
    return;
  }

  LOG_VERB("Check deadline.");

  if (deadline_.expires_at() <=
      boost::asio::deadline_timer::traits_type::now()) {
    // The deadline has passed.
    // The socket is closed so that any outstanding asynchronous operations
    // are canceled.
    LOG_WARN("HTTP client timed out.");
    Stop();
    timed_out_ = true;
  }

  // Put the actor back to sleep.
  deadline_.async_wait(std::bind(&HttpClient::CheckDeadline, this));
}

void HttpClient::Stop() {
  if (!stopped_) {
    stopped_ = true;

    LOG_INFO("Close socket...");

    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
      LOG_ERRO("Failed to close socket.");
    }

    deadline_.cancel();
  }
}

}  // namespace webcc
