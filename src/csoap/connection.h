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

class ConnectionManager;
class HttpRequestHandler;

// Represents a single connection from a client.
class Connection : public std::enable_shared_from_this<Connection> {
public:
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  // Construct a connection with the given io_service.
  Connection(boost::asio::ip::tcp::socket socket,
             ConnectionManager& manager,
             HttpRequestHandler& handler);

  // Start the first asynchronous operation for the connection.
  void Start();

  // Stop all asynchronous operations associated with the connection.
  void Stop();

private:
  void DoRead();

  void DoWrite();

  // Handle completion of a read operation.
  void HandleRead(boost::system::error_code ec,
                  std::size_t bytes_transferred);

  // Handle completion of a write operation.
  void HandleWrite(boost::system::error_code ec,
                   size_t bytes_transferred);

private:
  // Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  // The manager for this connection.
  ConnectionManager& connection_manager_;

  // The handler used to process the incoming request.
  HttpRequestHandler& request_handler_;

  // Buffer for incoming data.
  std::array<char, BUF_SIZE> buffer_;

  // The incoming request.
  HttpRequest request_;

  // The parser for the incoming request.
  HttpRequestParser request_parser_;

  // The reply to be sent back to the client.
  HttpResponse response_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

}  // namespace csoap

#endif  // CSOAP_CONNECTION_H_
