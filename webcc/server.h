#ifndef WEBCC_SERVER_H_
#define WEBCC_SERVER_H_

#include <string>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/signal_set.hpp"

#include "webcc/connection.h"
#include "webcc/connection_pool.h"
#include "webcc/request_handler.h"
#include "webcc/service.h"

namespace webcc {

// HTTP server accepts TCP connections from TCP clients.
// NOTE: Only support IPv4.
class Server {
public:
  Server(std::uint16_t port, std::size_t workers,
         const std::string& doc_root = "");

  virtual ~Server() = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  // Bind a service to the given URL path.
  // The URL should start with "/" and it will be treated as a regular
  // expression if |is_regex| is true.
  // Examples:
  //   - "/instances"
  //   - "/instances/(\\d+)"
  // Binding to the same URL multiple times is allowed, but only the last one
  // takes effect.
  bool Bind(ServicePtr service, const std::string& url, bool is_regex);

  // Run the server's io_service loop.
  void Run();

private:
  // Register to handle the signals that indicate when the server should exit.
  void RegisterSignals();

  // Initiate an asynchronous accept operation.
  void DoAccept();

  // Wait for a request to stop the server.
  void DoAwaitStop();

  // Get the handler for incoming requests.
  //virtual RequestHandler* GetRequestHandler();

  // The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;

  // Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  // The connection pool which owns all live connections.
  ConnectionPool pool_;

  // The signals for processing termination notifications.
  boost::asio::signal_set signals_;

  // The number of worker threads.
  std::size_t workers_;

  // The handler for incoming requests.
  RequestHandler request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_SERVER_H_
