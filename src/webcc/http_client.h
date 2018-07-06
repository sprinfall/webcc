#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include <cassert>
#include <memory>
#include <vector>

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
    assert(timeout_seconds > 0);
    timeout_seconds_ = timeout_seconds;
  }

  HttpResponsePtr response() const { return response_; }

  bool timed_out() const { return timed_out_; }

  Error error() const { return error_; }

  // Connect to server, send request, wait until response is received.
  bool Request(const HttpRequest& request);

 private:
  // Terminate all the actors to shut down the connection.
  void Stop();

  Error Connect(const HttpRequest& request);

  Error SendReqeust(const HttpRequest& request);

  Error ReadResponse();

  void DoReadResponse(Error* error);

  void CheckDeadline();

  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket socket_;

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

#endif  // WEBCC_HTTP_CLIENT_H_
