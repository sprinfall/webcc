#ifndef WEBCC_HTTP_ASYNC_CLIENT_H_
#define WEBCC_HTTP_ASYNC_CLIENT_H_

#include <array>

#include "boost/smart_ptr/scoped_ptr.hpp"

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "webcc/common.h"
#include "webcc/http_response_parser.h"

namespace webcc {

class HttpRequest;
class HttpResponse;

typedef void(*HttpResponseHandler)(std::shared_ptr<HttpResponse>);

class HttpAsyncClient {
public:
  HttpAsyncClient(boost::asio::io_context& io_context);

  Error SendRequest(std::shared_ptr<HttpRequest> request,
                    HttpResponseHandler response_handler);

private:
  void HandleResolve(boost::system::error_code ec,
                     boost::asio::ip::tcp::resolver::results_type results);

  void DoConnect(boost::asio::ip::tcp::resolver::results_type::iterator endpoint_it);

  void HandleConnect(boost::system::error_code ec,
                     boost::asio::ip::tcp::resolver::results_type::iterator endpoint_it);

  void DoWrite();

  void HandleWrite(boost::system::error_code ec);

  void DoRead();

  void HandleRead(boost::system::error_code ec, std::size_t length);

private:
  boost::asio::ip::tcp::socket socket_;

  std::shared_ptr<HttpRequest> request_;

  std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_;
  boost::asio::ip::tcp::resolver::results_type endpoints_;

  std::array<char, kBufferSize> buffer_;

  std::unique_ptr<HttpResponseParser> parser_;

  std::shared_ptr<HttpResponse> response_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_H_
