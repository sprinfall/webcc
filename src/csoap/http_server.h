#ifndef CSOAP_HTTP_SERVER_H_
#define CSOAP_HTTP_SERVER_H_

#include <string>
#include <vector>

#include "boost/asio/io_context.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "csoap/connection.h"
#include "csoap/connection_manager.h"
#include "csoap/http_request_handler.h"

namespace csoap {

// The top-level class of the HTTP server.
class HttpServer {
public:
  HttpServer(const HttpServer&) = delete;
  HttpServer& operator=(const HttpServer&) = delete;

  // Construct the server to listen on the specified TCP address and port, and
  // serve up files from the given directory.
  HttpServer(const std::string& address,
             const std::string& port);

  bool RegisterService(SoapServicePtr soap_service);

  // Run the server's io_service loop.
  void Run();

private:
  // Initiate an asynchronous accept operation.
  void DoAccept();

  // Wait for a request to stop the server.
  void DoAwaitStop();

private:
  // The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;

  // The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  // Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  // The connection manager which owns all live connections.
  ConnectionManager connection_manager_;

  // The handler for all incoming requests.
  HttpRequestHandler request_handler_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_SERVER_H_
