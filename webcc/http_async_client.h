#ifndef WEBCC_HTTP_ASYNC_CLIENT_H_
#define WEBCC_HTTP_ASYNC_CLIENT_H_

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

// Response handler/callback.
typedef std::function<void(HttpResponsePtr, Error, bool)> HttpResponseHandler;

// HTTP client session in asynchronous mode.
// A request will return without waiting for the response, the callback handler
// will be invoked when the response is received or timeout occurs.
// Don't use the same HttpAsyncClient object in multiple threads.
class HttpAsyncClient : public std::enable_shared_from_this<HttpAsyncClient> {
 public:
  explicit HttpAsyncClient(boost::asio::io_context& io_context);

  DELETE_COPY_AND_ASSIGN(HttpAsyncClient);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  // Asynchronously connect to the server, send the request, read the response,
  // and call the |response_handler| when all these finish.
  void Request(HttpRequestPtr request, HttpResponseHandler response_handler);

  // Terminate all the actors to shut down the connection. It may be called by
  // the user of the client class, or by the class itself in response to
  // graceful termination or an unrecoverable error.
  void Stop();

 private:
  using tcp = boost::asio::ip::tcp;

  void OnResolve(boost::system::error_code ec,
                 tcp::resolver::results_type results);

  void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint);

  void DoWrite();
  void OnWrite(boost::system::error_code ec);

  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void DoWaitDeadline();
  void OnDeadline(boost::system::error_code ec);

  tcp::resolver resolver_;
  tcp::socket socket_;

  std::shared_ptr<HttpRequest> request_;
  std::vector<char> buffer_;

  HttpResponsePtr response_;
  std::unique_ptr<HttpResponseParser> response_parser_;
  HttpResponseHandler response_handler_;

  // Timer for the timeout control.
  boost::asio::deadline_timer deadline_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  bool stopped_;
  bool timed_out_;
};

typedef std::shared_ptr<HttpAsyncClient> HttpAsyncClientPtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_H_
