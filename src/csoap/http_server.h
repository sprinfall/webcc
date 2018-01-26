#ifndef CSOAP_HTTP_SERVER_H_
#define CSOAP_HTTP_SERVER_H_

#include <string>
#include <vector>

#include "boost/scoped_ptr.hpp"
#include "boost/thread/thread.hpp"

#include "boost/asio/io_context.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/asio/ip/tcp.hpp"

#include "csoap/connection.h"
#include "csoap/http_request_handler.h"

namespace csoap {

// HTTP server accepts TCP connections from TCP clients.
// NOTE: Only support IPv4.
class HttpServer {
public:
  HttpServer(const HttpServer&) = delete;
  HttpServer& operator=(const HttpServer&) = delete;

  HttpServer(unsigned short port, std::size_t workers);

  bool RegisterService(SoapServicePtr soap_service);

  // Run the server's io_service loop.
  void Run();

private:
  // Initiate an asynchronous accept operation.
  void DoAccept();

  // Wait for a request to stop the server.
  void DoAwaitStop();

private:
  // The number of worker threads.
  std::size_t workers_;

  // The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;

  // The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  // Acceptor used to listen for incoming connections.
  boost::scoped_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

  // The handler for all incoming requests.
  HttpRequestHandler request_handler_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_SERVER_H_
