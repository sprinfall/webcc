#ifndef WEBCC_HTTP_ASYNC_CLIENT_BASE_H_
#define WEBCC_HTTP_ASYNC_CLIENT_BASE_H_

#include <functional>
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

// Response callback.
typedef std::function<void(HttpResponsePtr, Error, bool)> HttpResponseCallback;

// HTTP client session in asynchronous mode.
// A request will return without waiting for the response, the callback handler
// will be invoked when the response is received or timeout occurs.
// Don't use the same HttpAsyncClient object in multiple threads.
class HttpAsyncClientBase
    : public std::enable_shared_from_this<HttpAsyncClientBase> {
 public:
  // The |buffer_size| is the bytes of the buffer for reading response.
  // 0 means default value (e.g., 1024) will be used.
  explicit HttpAsyncClientBase(boost::asio::io_context& io_context,
                               std::size_t buffer_size = 0);

  virtual ~HttpAsyncClientBase() = default;

  WEBCC_DELETE_COPY_ASSIGN(HttpAsyncClientBase);

  // Set the timeout seconds for reading response.
  // The |seconds| is only effective when greater than 0.
  void SetTimeout(int seconds);

  // Asynchronously connect to the server, send the request, read the response,
  // and call the |response_callback| when all these finish.
  void Request(HttpRequestPtr request, HttpResponseCallback response_callback);

  // Called by the user to cancel the request.
  void Stop();

 protected:
  using tcp = boost::asio::ip::tcp;

  typedef tcp::resolver::results_type Endpoints;

  typedef std::function<void(boost::system::error_code, std::size_t)>
      ReadHandler;

  typedef std::function<void(boost::system::error_code, tcp::endpoint)>
      ConnectHandler;

  typedef std::function<void(boost::system::error_code, std::size_t)>
      WriteHandler;

  // To enable_shared_from_this for both parent and derived.
  // See https://stackoverflow.com/q/657155/6825348
  template <typename Derived>
  std::shared_ptr<Derived> shared_from_base() {
    return std::static_pointer_cast<Derived>(shared_from_this());
  }

 protected:
  virtual void Resolve() = 0;

  void DoResolve(const std::string& default_port);
  void OnResolve(boost::system::error_code ec,
                 tcp::resolver::results_type results);

  void DoConnect(const Endpoints& endpoints);
  void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint);

  virtual void OnConnected() = 0;

  void DoWrite();
  void OnWrite(boost::system::error_code ec, std::size_t length);

  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void DoWaitDeadline();
  void OnDeadline(boost::system::error_code ec);

  // Terminate all the actors to shut down the connection. 
  void DoStop();

  virtual void SocketAsyncConnect(const Endpoints& endpoints,
                                  ConnectHandler&& handler) = 0;

  virtual void SocketAsyncWrite(WriteHandler&& handler) = 0;

  virtual void SocketAsyncReadSome(ReadHandler&& handler) = 0;

  virtual void SocketClose(boost::system::error_code* ec) = 0;

  tcp::resolver resolver_;

  HttpRequestPtr request_;

  std::vector<char> buffer_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;
  HttpResponseCallback response_callback_;

  // Timer for the timeout control.
  boost::asio::deadline_timer deadline_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  // Request stopped due to timeout or socket error.
  bool stopped_;

  // If the error was caused by timeout or not.
  // Will be passed to the response handler/callback.
  bool timed_out_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_BASE_H_
