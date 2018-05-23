#ifndef WEBCC_HTTP_SESSION_H_
#define WEBCC_HTTP_SESSION_H_

#include <array>
#include <memory>
#include <string>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "webcc/globals.h"
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

  ~HttpSession() = default;

  const HttpRequest& request() const {
    return request_;
  }

  // Start to read and process the client request.
  void Start();

  // Close the socket.
  void Close();

  void SetResponseContent(std::string&& content,
                          const std::string& content_type);

  // Send response to client with the given status.
  void SendResponse(HttpStatus::Enum status);

 private:
  void AsyncRead();
  void ReadHandler(boost::system::error_code ec, std::size_t length);

  void AsyncWrite();
  void WriteHandler(boost::system::error_code ec, std::size_t length);

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
