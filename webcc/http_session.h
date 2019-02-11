#ifndef WEBCC_HTTP_SESSION_H_
#define WEBCC_HTTP_SESSION_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_request_parser.h"
#include "webcc/http_response.h"

namespace webcc {

class HttpRequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
 public:
  HttpSession(boost::asio::ip::tcp::socket socket,
              HttpRequestHandler* handler);

  ~HttpSession() = default;

  WEBCC_DELETE_COPY_ASSIGN(HttpSession);

  const HttpRequest& request() const { return request_; }

  // Start to read and process the client request.
  void Start();

  // Close the socket.
  void Close();

  void SetResponseContent(std::string&& content,
                          const std::string& media_type,
                          const std::string& charset);

  // Send response to client with the given status.
  void SendResponse(http::Status status);

 private:
  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void DoWrite();
  void OnWrite(boost::system::error_code ec, std::size_t length);

  // Shutdown the socket.
  void Shutdown();

  // Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // Buffer for incoming data.
  std::vector<char> buffer_;

  // The handler used to process the incoming request.
  HttpRequestHandler* request_handler_;

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
