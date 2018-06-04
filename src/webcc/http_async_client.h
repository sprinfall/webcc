#ifndef WEBCC_HTTP_ASYNC_CLIENT_H_
#define WEBCC_HTTP_ASYNC_CLIENT_H_

#include <array>
#include <functional>
#include <memory>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/http_response_parser.h"

namespace webcc {

typedef std::function<void(HttpResponsePtr, Error)> HttpResponseHandler;

class HttpAsyncClient : public std::enable_shared_from_this<HttpAsyncClient> {
 public:
  explicit HttpAsyncClient(boost::asio::io_context& io_context);

  DELETE_COPY_AND_ASSIGN(HttpAsyncClient);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  // Asynchronously connect to the server, send the request, read the response,
  // and call the |response_handler| when all these finish.
  Error Request(HttpRequestPtr request, HttpResponseHandler response_handler);

 private:
  using tcp = boost::asio::ip::tcp;

  void ResolveHandler(boost::system::error_code ec,
                      tcp::resolver::results_type results);

  void AsyncConnect(tcp::resolver::results_type::iterator endpoint_it);

  void ConnectHandler(boost::system::error_code ec,
                      tcp::resolver::results_type::iterator endpoint_it);

  void AsyncWrite();
  void WriteHandler(boost::system::error_code ec);

  void AsyncRead();
  void ReadHandler(boost::system::error_code ec, std::size_t length);

  void CheckDeadline();

  tcp::socket socket_;

  std::shared_ptr<HttpRequest> request_;

  std::unique_ptr<tcp::resolver> resolver_;
  tcp::resolver::results_type endpoints_;

  std::array<char, kBufferSize> buffer_;

  std::unique_ptr<HttpResponseParser> response_parser_;

  HttpResponsePtr response_;
  HttpResponseHandler response_handler_;

  // Maximum seconds to wait before the client cancels the operation.
  // Only for receiving response from server.
  int timeout_seconds_;

  // Timer for the timeout control.
  boost::asio::deadline_timer deadline_timer_;
};

typedef std::shared_ptr<HttpAsyncClient> HttpAsyncClientPtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_H_
