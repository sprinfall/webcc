#ifndef WEBCC_CLIENT_H_
#define WEBCC_CLIENT_H_

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/steady_timer.hpp"

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/response_parser.h"
#include "webcc/socket.h"

namespace webcc {

class Client;
using ClientPtr = std::shared_ptr<Client>;

// Synchronous HTTP & HTTPS client.
// In synchronous mode, a request won't return until the response is received
// or timeout occurs.
// Please don't use the same client object in multiple threads.
class Client {
public:
  Client();

  virtual ~Client() = default;

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  void set_ssl_verify(bool ssl_verify) {
    ssl_verify_ = ssl_verify;
  }

  void set_buffer_size(std::size_t buffer_size) {
    if (buffer_size > 0) {
      buffer_size_ = buffer_size;
    }
  }

  // Set the timeout (in seconds) for reading response.
  void set_timeout(int timeout)  {
    if (timeout > 0) {
      timeout_ = timeout;
    }
  }

  // Connect to server, send request, wait until response is received.
  bool Request(RequestPtr request, bool connect = true);

  // Close the socket.
  void Close();

  ResponsePtr response() const { return response_; }

  bool closed() const { return closed_; }

  bool timed_out() const { return timed_out_; }

  Error error() const { return error_; }

private:
  Error Connect(RequestPtr request);

  Error DoConnect(RequestPtr request, const std::string& default_port);

  Error WriteReqeust(RequestPtr request);

  Error ReadResponse();

  void DoReadResponse(Error* error);

  void DoWaitTimer();
  void OnTimer(boost::system::error_code ec);

  // Cancel any async-operations waiting on the timer.
  void CancelTimer();

private:
  boost::asio::io_context io_context_;

  // Socket connection.
  std::unique_ptr<SocketBase> socket_;

  ResponsePtr response_;
  ResponseParser response_parser_;

  // Timer for the timeout control.
  boost::asio::steady_timer timer_;

  // The buffer for reading response.
  std::vector<char> buffer_;

  // Verify the certificate of the peer or not (for HTTPS).
  bool ssl_verify_;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_;

  // Timeout (seconds) for receiving response.
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

#endif  // WEBCC_CLIENT_H_
