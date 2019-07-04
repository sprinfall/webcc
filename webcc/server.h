#ifndef WEBCC_SERVER_H_
#define WEBCC_SERVER_H_

#include <regex>
#include <string>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/signal_set.hpp"

#include "webcc/connection.h"
#include "webcc/connection_pool.h"
#include "webcc/request_handler.h"
#include "webcc/view.h"

namespace webcc {

class Server {
public:
  explicit Server(std::uint16_t port, std::size_t workers = 1,
                  const Path& doc_root = {});

  virtual ~Server() = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  // Route a URL to a view.
  // The URL should start with "/". E.g., "/instances".
  bool Route(const std::string& url, ViewPtr view,
             const Strings& methods = { "GET" }) {
    return request_handler_.Route(url, view, methods);
  }

  // Route a regular expression URL to a view.
  // The URL should start with "/" and be a regular expression.
  // E.g., "/instances/(\\d+)".
  bool Route(const RegexUrl& regex_url, ViewPtr view,
             const Strings& methods = { "GET" }) {
    return request_handler_.Route(regex_url, view, methods);
  }

  // Run the loop.
  void Run();

private:
  // Register to handle the signals that indicate when the server should exit.
  void RegisterSignals();

  // Initiate an asynchronous accept operation.
  void DoAccept();

  // Wait for a request to stop the server.
  void DoAwaitStop();

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
