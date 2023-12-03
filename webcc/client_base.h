#ifndef WEBCC_CLIENT_BASE_H_
#define WEBCC_CLIENT_BASE_H_

// HTTP client base class.

#include <atomic>
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

class ClientBase : public std::enable_shared_from_this<ClientBase> {
public:
  ClientBase(boost::asio::io_context& io_context,
             std::string_view default_port);

  ClientBase(const ClientBase&) = delete;
  ClientBase& operator=(const ClientBase&) = delete;

  virtual ~ClientBase() = default;

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

  void set_read_timeout(int timeout) {
    if (timeout > 0) {
      read_timeout_ = timeout;
    }
  }

  void set_subsequent_read_timeout(int timeout) {
    if (timeout > 0) {
      subsequent_read_timeout_ = timeout;
    }
  }

  // Set progress callback to be informed about the read/write progress.
  // NOTE: Don't use move semantics because in practice, there is no difference
  //       between copying and moving an object of a closure type.
  void set_progress_callback(ProgressCallback callback) {
    progress_callback_ = callback;
  }

  // Close the connection (shut down and close socket).
  // The async operation on the socket will be canceled.
  virtual bool Close();

  bool connected() const {
    return connected_;
  }

  RequestPtr request() const {
    return request_;
  }

  // Send a request to the server.
  void Send(RequestPtr request, bool stream = false);

  ResponsePtr response() const {
    return response_;
  }

  Error error() const {
    return error_;
  }

  // Reset response object.
  // Used to make sure the response object will be released even the client
  // object itself will be cached for keep-alive purpose.
  void Reset() {
    response_.reset();
    response_parser_.Init(nullptr, false);
  }

protected:
  // Get underlying socket.
  virtual SocketType& GetSocket() = 0;

  virtual void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                          AsyncRWHandler&& handler) = 0;

  virtual void AsyncReadSome(boost::asio::mutable_buffer buffer,
                             AsyncRWHandler&& handler) = 0;

  // Request begin.
  // Initialize the states here (e.g., reset error and flags).
  virtual void RequestBegin() {
    error_.Clear();
  }

  // The current request has ended.
  //virtual void RequestEnd() = 0;

  virtual void OnConnected() {
    AsyncWrite();
  }

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

  // The buffer for reading response.
  std::vector<char> buffer_;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_ = kBufferSize;

  // Timeout (seconds) for connecting to server.
  // Default as 0 to disable our own control (i.e., deadline_timer_).
  int connect_timeout_ = 0;

  // Timeout (seconds) for reading response.
  int read_timeout_ = 30;

  // Timeout (seconds) for each subsequent read during reading a response.
  // A reasonable value should be much less than `read_timeout_`.
  int subsequent_read_timeout_ = 10;

  // Deadline timer for connecting to server and reading respone.
  boost::asio::steady_timer deadline_timer_;
  bool deadline_timer_stopped_ = true;

  // Socket connected or not.
  std::atomic_bool connected_ = false;

  // Progress callback (optional).
  ProgressCallback progress_callback_;

  // The length already read/written.
  std::size_t current_length_ = 0;

  std::size_t total_length_ = 0;

  // Current error.
  Error error_;
};

using ClientPtr = std::shared_ptr<ClientBase>;

}  // namespace webcc

#endif  // WEBCC_CLIENT_BASE_H_
