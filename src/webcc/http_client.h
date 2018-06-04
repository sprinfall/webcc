#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include <array>
#include <memory>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/http_response_parser.h"

namespace webcc {

class HttpClient {
 public:
  HttpClient();

  ~HttpClient() = default;

  DELETE_COPY_AND_ASSIGN(HttpClient);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  HttpResponsePtr response() const { return response_; }

  Error error() const { return error_; }

  // Connect to server, send request, wait until response is received.
  bool Request(const HttpRequest& request);

 private:
  Error Connect(const HttpRequest& request);

  Error SendReqeust(const HttpRequest& request);

  Error ReadResponse();

  void CheckDeadline();

  boost::asio::io_context io_context_;

  boost::asio::ip::tcp::socket socket_;

  std::array<char, kBufferSize> buffer_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;

  Error error_ = kNoError;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  // Timer for the timeout control.
  boost::asio::deadline_timer deadline_timer_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
