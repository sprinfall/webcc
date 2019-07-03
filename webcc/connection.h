#ifndef WEBCC_CONNECTION_H_
#define WEBCC_CONNECTION_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/ip/tcp.hpp"  // for ip::tcp::socket

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/request_parser.h"
#include "webcc/response.h"

namespace webcc {

class Connection;
class ConnectionPool;
class RequestHandler;

using ConnectionPtr = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection> {
public:
  Connection(boost::asio::ip::tcp::socket socket, ConnectionPool* pool,
             RequestHandler* handler);

  ~Connection() = default;

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  RequestPtr request() const {
    return request_;
  }

  // Start to read and process the client request.
  void Start();

  // Close the socket.
  void Close();

  // Send response to client.
  void SendResponse(ResponsePtr response);

  // TODO: Remove
  void SendResponse(Status status);

private:
  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void DoWrite();
  void OnWriteHeaders(boost::system::error_code ec, std::size_t length);
  void DoWriteBody();
  void OnWriteBody(boost::system::error_code ec, std::size_t length);
  void OnWriteOK();
  void OnWriteError(boost::system::error_code ec);

  // Shutdown the socket.
  void Shutdown();

  // Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // The pool for this connection.
  ConnectionPool* pool_;

  // Buffer for incoming data.
  std::vector<char> buffer_;

  // The handler used to process the incoming request.
  RequestHandler* request_handler_;

  // The incoming request.
  RequestPtr request_;

  // The parser for the incoming request.
  RequestParser request_parser_;

  // The response to be sent back to the client.
  ResponsePtr response_;
};

}  // namespace webcc

#endif  // WEBCC_CONNECTION_H_
