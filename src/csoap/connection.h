#ifndef CSOAP_CONNECTION_H_
#define CSOAP_CONNECTION_H_

#include <array>
#include <memory>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "csoap/common.h"
#include "csoap/http_request.h"
#include "csoap/http_request_parser.h"
#include "csoap/http_response.h"

namespace csoap {

class HttpRequestHandler;

// Represents a single connection from a client.
class Connection : public std::enable_shared_from_this<Connection> {
public:
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  friend class HttpRequestHandler;

  // Construct a connection with the given io_service.
  Connection(boost::asio::ip::tcp::socket socket,
             HttpRequestHandler& handler);

  void Start();

  void Stop();

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
  HttpRequestHandler& request_handler_;

  // Buffer for incoming data.
  std::array<char, BUF_SIZE> buffer_;

  // The incoming request.
  HttpRequest request_;

  // The parser for the incoming request.
  HttpRequestParser request_parser_;

  // The response to be sent back to the client.
  HttpResponse response_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

}  // namespace csoap

#endif  // CSOAP_CONNECTION_H_
