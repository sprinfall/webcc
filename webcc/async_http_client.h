#ifndef WEBCC_ASYNC_HTTP_CLIENT_H_
#define WEBCC_ASYNC_HTTP_CLIENT_H_

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

// Request handler/callback.
typedef std::function<void(HttpResponsePtr, Error, bool)> HttpResponseHandler;

class AsyncHttpClient : public std::enable_shared_from_this<AsyncHttpClient> {
 public:
  explicit AsyncHttpClient(boost::asio::io_context& io_context);

  DELETE_COPY_AND_ASSIGN(AsyncHttpClient);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  // Asynchronously connect to the server, send the request, read the response,
  // and call the |response_handler| when all these finish.
  Error Request(HttpRequestPtr request, HttpResponseHandler response_handler);

  // Terminate all the actors to shut down the connection. It may be called by
  // the user of the client class, or by the class itself in response to
  // graceful termination or an unrecoverable error.
  void Stop();

 private:
  using tcp = boost::asio::ip::tcp;
  typedef tcp::resolver::results_type::iterator EndpointIterator;

  void ResolveHandler(boost::system::error_code ec,
                      tcp::resolver::results_type results);

  void AsyncConnect(EndpointIterator endpoint_iter);

  void ConnectHandler(boost::system::error_code ec,
                      EndpointIterator endpoint_iter);

  void AsyncWrite();
  void WriteHandler(boost::system::error_code ec);

  void AsyncRead();
  void ReadHandler(boost::system::error_code ec, std::size_t length);

  void CheckDeadline();

  tcp::socket socket_;
  std::unique_ptr<tcp::resolver> resolver_;
  tcp::resolver::results_type endpoints_;

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

typedef std::shared_ptr<AsyncHttpClient> HttpAsyncClientPtr;

}  // namespace webcc

#endif  // WEBCC_ASYNC_HTTP_CLIENT_H_
