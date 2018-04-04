#ifndef CSOAP_HTTP_SESSION_H_
#define CSOAP_HTTP_SESSION_H_

#include <array>
#include <memory>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "csoap/common.h"
#include "csoap/http_request.h"
#include "csoap/http_request_parser.h"
#include "csoap/http_response.h"

namespace csoap {

class HttpRequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
  friend class HttpRequestHandler;

  HttpSession(const HttpSession&) = delete;
  HttpSession& operator=(const HttpSession&) = delete;

  HttpSession(boost::asio::ip::tcp::socket socket,
              HttpRequestHandler* handler);

  const HttpRequest& request() const {
    return request_;
  }

  void Start();

  void Stop();

  void SetResponseStatus(int status) {
    response_.set_status(status);
  }

  void SetResponseContent(const std::string& content_type,
                          std::size_t content_length,
                          std::string&& content);

  // Write response back to the client.
  void SendResponse();

private:
  void DoRead();

  void DoWrite();

  void HandleRead(boost::system::error_code ec,
                  std::size_t length);

  void HandleWrite(boost::system::error_code ec,
                   std::size_t length);

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

}  // namespace csoap

#endif  // CSOAP_HTTP_SESSION_H_
