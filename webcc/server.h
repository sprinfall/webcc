#ifndef WEBCC_SERVER_H_
#define WEBCC_SERVER_H_

#include <string>
#include <thread>
#include <vector>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/signal_set.hpp"

#include "webcc/connection.h"
#include "webcc/connection_pool.h"
#include "webcc/queue.h"
#include "webcc/url.h"
#include "webcc/view.h"

namespace webcc {

class Server {
public:
  explicit Server(std::uint16_t port, const Path& doc_root = {});

  virtual ~Server() = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  // Route a URL to a view.
  // The URL should start with "/". E.g., "/instances".
  bool Route(const std::string& url, ViewPtr view,
             const Strings& methods = { "GET" });

  // Route a URL (as regular expression) to a view.
  // The URL should start with "/" and be a regular expression.
  // E.g., "/instances/(\\d+)".
  bool Route(const UrlRegex& regex_url, ViewPtr view,
             const Strings& methods = { "GET" });

  // Start the server with a given number of worker threads.
  void Start(std::size_t workers = 1);

  // Stop the server.
  void Stop();

  // Put the connection into the queue.
  void Enqueue(ConnectionPtr connection);

private:
  // Register to handle the signals that indicate when the server should exit.
  void RegisterSignals();

  // Initiate an asynchronous accept operation.
  void DoAccept();

  // Wait for a request to stop the server.
  void DoAwaitStop();

  // Worker thread routine.
  void WorkerRoutine();

  // Clear pending connections from the queue and stop worker threads.
  void StopWorkers();

  // Handle a connection (or more precisely, the request inside it).
  // Get the request from the connection, process it, prepare the response,
  // then send the response back to the client.
  // The connection will keep alive if it's a persistent connection. When next
  // request comes, this connection will be put back to the queue again.
  virtual void Handle(ConnectionPtr connection);

  // Find the view by HTTP method and URL.
  ViewPtr FindView(const std::string& method, const std::string& url,
                   UrlArgs* args);

  // Serve static files from the doc root.
  bool ServeStatic(ConnectionPtr connection);

private:
  struct RouteInfo {
    std::string url;
    std::regex url_regex;
    ViewPtr view;
    Strings methods;
  };

  // The io_context used to perform asynchronous operations.
  boost::asio::io_context io_context_;

  // Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  // The connection pool which owns all live connections.
  ConnectionPool pool_;

  // The signals for processing termination notifications.
  boost::asio::signal_set signals_;

  // Worker threads.
  std::vector<std::thread> worker_threads_;

  // The directory with the static files to be served.
  Path doc_root_;

  // The queue with connection waiting for the workers to process.
  Queue<ConnectionPtr> queue_;

  // Route table.
  std::vector<RouteInfo> routes_;
};

}  // namespace webcc

#endif  // WEBCC_SERVER_H_
