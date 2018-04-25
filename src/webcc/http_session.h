#ifndef WEBCC_HTTP_SESSION_H_
#define WEBCC_HTTP_SESSION_H_

#include <array>
#include <memory>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "webcc/common.h"
#include "webcc/http_request.h"
#include "webcc/http_request_parser.h"
#include "webcc/http_response.h"

namespace webcc {

class HttpRequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
  HttpSession(const HttpSession&) = delete;
  HttpSession& operator=(const HttpSession&) = delete;

  HttpSession(boost::asio::ip::tcp::socket socket,
              HttpRequestHandler* handler);

  ~HttpSession();

  const HttpRequest& request() const {
    return request_;
  }

  void Start();

  void Stop();

  void SetResponseContent(std::string&& content,
                          const std::string& content_type);

  // Write response back to the client with the given HTTP status.
  void SendResponse(int status);

private:
  void DoRead();

  void DoWrite();

  void HandleRead(boost::system::error_code ec, std::size_t length);
  void HandleWrite(boost::system::error_code ec, std::size_t length);

private:
  // Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // The handler used to process the incoming request.
  HttpRequestHandler* request_handler_;

  // Buffer for incoming data.
  std::array<char, kBufferSize> buffer_;

  // The incoming request.
  HttpRequest request_;

  // The parser for the incoming request.
  HttpRequestParser request_parser_;

  // The response to be sent back to the client.
  HttpResponse response_;
};

typedef std::shared_ptr<HttpSession> HttpSessionPtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_SESSION_H_
