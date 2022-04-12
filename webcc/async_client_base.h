#ifndef WEBCC_ASYNC_CLIENT_BASE_H_
#define WEBCC_ASYNC_CLIENT_BASE_H_

// Asynchronous HTTP client base class.

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

namespace webcc {

using SocketType = boost::asio::basic_socket<boost::asio::ip::tcp,
                                             boost::asio::any_io_executor>;

class AsyncClientBase : public std::enable_shared_from_this<AsyncClientBase> {
public:
  AsyncClientBase(boost::asio::io_context& io_context,
                  std::string_view default_port);

  AsyncClientBase(const AsyncClientBase&) = delete;
  AsyncClientBase& operator=(const AsyncClientBase&) = delete;

  virtual ~AsyncClientBase() = default;

  void set_buffer_size(std::size_t buffer_size) {
    if (buffer_size > 0) {
      buffer_size_ = buffer_size;
    }
  }

  void set_connect_timeout(int timeout) {
    if (timeout > 0) {
      connect_timeout_ = timeout;
    }
  }

  void set_read_timeout(int timeout)  {
    if (timeout > 0) {
      read_timeout_ = timeout;
    }
  }

  // Set progress callback to be informed about the read progress.
  // NOTE: Don't use move semantics because in practice, there is no difference
  //       between copying and moving an object of a closure type.
  // TODO: Support write progress
  void set_progress_callback(ProgressCallback callback) {
    progress_callback_ = callback;
  }

  // Close the connection.
  // The async operation on the socket will be canceled.
  void Close() {
    CloseSocket();
  }

  bool connected() const {
    return connected_;
  }

  RequestPtr request() const {
    return request_;
  }

  ResponsePtr response() const {
    return response_;
  }

  Error error() const {
    return error_;
  }

  // Reset response object.
  // Used to make sure the response object will released even the client object
  // itself will be cached for keep-alive purpose.
  void Reset() {
    response_.reset();
    response_parser_.Init(nullptr, false);
  }

protected:
  // Read/write handler
  using RWHandler = std::function<void(boost::system::error_code, std::size_t)>;

  // Get underlying socket.
  virtual SocketType& GetSocket() = 0;

  virtual void OnConnected() {
    AsyncWrite();
  }

  // The current request has ended.
  virtual void RequestEnd() = 0;

  virtual void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                          RWHandler&& handler) = 0;

  virtual void AsyncReadSome(boost::asio::mutable_buffer buffer,
                             RWHandler&& handler) = 0;

  // Shutdown and close socket.
  virtual void CloseSocket();

  // Send a request to the server.
  // Check `error()` for any error.
  void AsyncSend(RequestPtr request, bool stream = false);

  void AsyncResolve();

  void OnResolve(boost::system::error_code ec,
                 boost::asio::ip::tcp::resolver::results_type endpoints);

  void OnConnect(boost::system::error_code ec,
                 boost::asio::ip::tcp::endpoint endpoint);

  void AsyncWrite();
  void OnWrite(boost::system::error_code ec, std::size_t length);

  void AsyncWriteBody();
  void OnWriteBody(boost::system::error_code ec, std::size_t length);

  void HandleWriteError(boost::system::error_code ec);

  void AsyncRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void AsyncWaitDeadlineTimer(int seconds);
  void OnDeadlineTimer(boost::system::error_code ec);
  void StopDeadlineTimer();

protected:
  boost::asio::io_context& io_context_;

  // The default port used to resolve when the URL doesn't have one.
  // E.g., "80" for HTTP and "443" for HTTPS.
  const std::string default_port_;

  boost::asio::ip::tcp::resolver resolver_;

  RequestPtr request_;

  ResponsePtr response_;
  ResponseParser response_parser_;

  // The length already read.
  std::size_t length_read_ = 0;

  // The buffer for reading response.
  std::vector<char> buffer_;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_ = kBufferSize;

  // Timeout (seconds) for connecting to server.
  // Default as 0 to disable our own control (i.e., deadline_timer_).
  int connect_timeout_ = 0;

  // Timeout (seconds) for reading response.
  int read_timeout_ = kMaxReadSeconds;

  // Deadline timer for connecting to server.
  boost::asio::steady_timer deadline_timer_;
  bool deadline_timer_stopped_ = true;

  // Socket connected or not.
  bool connected_ = false;

  // Progress callback (optional).
  ProgressCallback progress_callback_;

  // Current error.
  Error error_;
};

}  // namespace webcc

#endif  // WEBCC_ASYNC_CLIENT_BASE_H_
