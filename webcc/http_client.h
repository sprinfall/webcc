#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/http_response_parser.h"
#include "webcc/http_socket.h"

namespace webcc {

class HttpClient;
typedef std::shared_ptr<HttpClient> HttpClientPtr;

// Synchronous HTTP & HTTPS client.
// In synchronous mode, a request won't return until the response is received
// or timeout occurs.
// Please don't use the same client object in multiple threads.
class HttpClient {
public:
  explicit HttpClient(bool ssl_verify = true, std::size_t buffer_size = 0);

  virtual ~HttpClient() = default;

  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  void set_buffer_size(std::size_t buffer_size) {
    buffer_size_ = (buffer_size == 0 ? kBufferSize : buffer_size);
  }

  void set_ssl_verify(bool ssl_verify) {
    ssl_verify_ = ssl_verify;
  }

  // Set the timeout (in seconds) for reading response.
  void set_timeout(int timeout)  {
    if (timeout > 0) {
      timeout_ = timeout;
    }
  }

  // Connect to server, send request, wait until response is received.
  bool Request(const HttpRequest& request, bool connect = true);

  // Close the socket.
  void Close();

  HttpResponsePtr response() const { return response_; }

  int response_status() const {
    assert(response_);
    return response_->status();
  }

  const std::string& response_content() const {
    assert(response_);
    return response_->content();
  }

  bool closed() const { return closed_; }

  bool timed_out() const { return timed_out_; }

  Error error() const { return error_; }

private:
  Error Connect(const HttpRequest& request);

  Error DoConnect(const HttpRequest& request, const std::string& default_port);

  Error WriteReqeust(const HttpRequest& request);

  Error ReadResponse();

  void DoReadResponse(Error* error);

  void DoWaitTimer();
  void OnTimer(boost::system::error_code ec);

  // Cancel any async-operations waiting on the timer.
  void CancelTimer();

private:
  boost::asio::io_context io_context_;

  // Socket connection.
  std::unique_ptr<HttpSocketBase> socket_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;

  // Timer for the timeout control.
  boost::asio::deadline_timer timer_;

  // The buffer for reading response.
  std::vector<char> buffer_;

  // Verify the certificate of the peer or not (for HTTPS).
  bool ssl_verify_;

  // The size of the buffer for reading response.
  // Set 0 for using default value (e.g., 1024).
  std::size_t buffer_size_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for reading response from server.
  int timeout_;

  // Connection closed.
  bool closed_;

  // Deadline timer canceled.
  bool timer_canceled_;

  // Timeout occurred.
  bool timed_out_;

  // Error code.
  Error error_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
