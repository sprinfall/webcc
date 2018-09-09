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

// HTTP client session in synchronous mode.
// A request will not return until the response is received or timeout occurs.
// Don't use the same HttpClient object in multiple threads.
class HttpClient {
 public:
  HttpClient();
  ~HttpClient() = default;

  DELETE_COPY_AND_ASSIGN(HttpClient);

  // Set the timeout seconds for reading response.
  // The |seconds| is only effective when greater than 0.
  void SetTimeout(int seconds);

  // Connect to server, send request, wait until response is received.
  bool Request(const HttpRequest& request);

  HttpResponsePtr response() const { return response_; }

  bool timed_out() const { return timed_out_; }

  Error error() const { return error_; }

 private:
  Error Connect(const HttpRequest& request);

  Error SendReqeust(const HttpRequest& request);

  Error ReadResponse();

  void DoReadResponse(Error* error);

  void DoWaitDeadline();
  void OnDeadline(boost::system::error_code ec);

  void Stop();

  boost::asio::io_context io_context_;

  boost::asio::ip::tcp::socket socket_;

  std::vector<char> buffer_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;

  boost::asio::deadline_timer deadline_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for reading response from server.
  int timeout_seconds_;

  bool stopped_;

  // If the error was caused by timeout or not.
  bool timed_out_;

  Error error_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
