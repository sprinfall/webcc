#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include <array>
#include <memory>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/io_context.hpp"

#include "webcc/common.h"
#include "webcc/http_response_parser.h"

namespace webcc {

class HttpRequest;
class HttpResponse;

class HttpClient {
public:
  HttpClient();
  ~HttpClient() = default;

  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  // Make a HTTP request.
  // Connect to the server, send the request, wait until the response is
  // received.
  Error MakeRequest(const HttpRequest& request, HttpResponse* response);

private:
  Error Connect(const HttpRequest& request);

  Error SendReqeust(const HttpRequest& request);

  Error ReadResponse(HttpResponse* response);

  void CheckDeadline();

private:
  boost::asio::io_context io_context_;

  boost::asio::ip::tcp::socket socket_;

  std::array<char, kBufferSize> buffer_;

  std::unique_ptr<HttpResponseParser> parser_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  // Timer for the timeout control.
  boost::asio::deadline_timer deadline_timer_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
