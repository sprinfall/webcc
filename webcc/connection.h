#ifndef WEBCC_CONNECTION_H_
#define WEBCC_CONNECTION_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/request_parser.h"
#include "webcc/response.h"

namespace webcc {

class ConnectionPool;
class Server;

class Connection : public std::enable_shared_from_this<Connection> {
public:
  Connection(boost::asio::ip::tcp::socket socket, ConnectionPool* pool,
             Server* server);

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

  // Send a response to the client.
  // `Connection` header will be set to "Close" if |no_keep_alive| is true no
  // matter whether the client asked for Keep-Alive or not.
  void SendResponse(ResponsePtr response, bool no_keep_alive = false);

  // Send a response with the given status and an empty body to the client.
  // `Connection` header will be set to "Close" if |no_keep_alive| is true no
  // matter whether the client asked for Keep-Alive or not.
  void SendResponse(Status status, bool no_keep_alive = false);

private:
  void DoRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void DoWrite();
  void OnWriteHeaders(boost::system::error_code ec, std::size_t length);
  void DoWriteBody();
  void OnWriteBody(boost::system::error_code ec, std::size_t length);
  void OnWriteOK();
  void OnWriteError(boost::system::error_code ec);

  // The socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // The pool for this connection.
  ConnectionPool* pool_;

  // The buffer for incoming data.
  std::vector<char> buffer_;

  // The server.
  Server* server_;

  // The incoming request.
  RequestPtr request_;

  // The parser for the incoming request.
  RequestParser request_parser_;

  // The response to be sent back to the client.
  ResponsePtr response_;
};

using ConnectionPtr = std::shared_ptr<Connection>;

}  // namespace webcc

#endif  // WEBCC_CONNECTION_H_
