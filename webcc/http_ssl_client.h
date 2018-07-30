#ifndef WEBCC_HTTP_SSL_CLIENT_H_
#define WEBCC_HTTP_SSL_CLIENT_H_

#include <cassert>
#include <memory>
#include <vector>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/ssl.hpp"

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/http_response_parser.h"

namespace webcc {

class HttpSslClient {
 public:
  HttpSslClient();

  ~HttpSslClient() = default;

  DELETE_COPY_AND_ASSIGN(HttpSslClient);

  void set_timeout_seconds(int timeout_seconds) {
    assert(timeout_seconds > 0);
    timeout_seconds_ = timeout_seconds;
  }

  // Connect to server, send request, wait until response is received.
  bool Request(const HttpRequest& request);

  HttpResponsePtr response() const { return response_; }

  bool timed_out() const { return timed_out_; }

  Error error() const { return error_; }

private:
  Error Connect(const HttpRequest& request);

  Error Handshake(const std::string& host);

  Error SendReqeust(const HttpRequest& request);

  Error ReadResponse();

  void DoReadResponse(Error* error);

  void CheckDeadline();

  void Stop();

  boost::asio::io_context io_context_;

  boost::asio::ssl::context ssl_context_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

  std::vector<char> buffer_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;

  boost::asio::deadline_timer deadline_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  bool stopped_;

  // If the error was caused by timeout or not.
  bool timed_out_;

  Error error_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_SSL_CLIENT_H_
