#ifndef WEBCC_SERVER_H_
#define WEBCC_SERVER_H_

#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/signal_set.hpp"

#include "webcc/connection.h"
#include "webcc/connection_pool.h"
#include "webcc/queue.h"
#include "webcc/router.h"
#include "webcc/url.h"

namespace webcc {

class Server : public Router {
public:
  explicit Server(std::uint16_t port,
                  const std::filesystem::path& doc_root = {});

  ~Server() = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  void set_file_chunk_size(std::size_t file_chunk_size) {
    assert(file_chunk_size > 0);
    file_chunk_size_ = file_chunk_size;
  }

  // Start and run the server.
  // This method is blocking so will not return until Stop() is called (from
  // another thread) or a signal like SIGINT is caught.
  // When the request of a connection has been read, the connection is put into
  // a queue waiting for some worker thread to process. Normally, the more
  // |workers| you have, the more concurrency you gain (the concurrency also
  // depends on the number of CPU cores). The worker thread pops connections
  // from the queue one by one, prepares the response by the user provided View,
  // then sends it back to the client.
  // Meanwhile, the (event) loop, i.e., io_context, is also running in a number
  // (|loops|) of threads. Normally, one thread for the loop is good enough, but
  // it could be more than that.
  void Run(std::size_t workers = 1, std::size_t loops = 1);

  // Stop the server.
  // This should be called from another thread since the Run() is blocking.
  void Stop();

  // Is the server running?
  bool IsRunning() const;

private:
  // Register signals which indicate when the server should exit.
  void AddSignals();

  // Wait for a signal to stop the server.
  void AsyncWaitSignals();

  // Listen on the given port.
  bool Listen(std::uint16_t port);

  // Accept connections asynchronously.
  void AsyncAccept();

  // Stop acceptor and worker threads, close all pending connections, and
  // finally stop the event loop.
  void DoStop();

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
 
  // Match the view by HTTP method and URL (path).
  // Return if a view or static file is matched or not.
  // If the view asks for data streaming, |stream| will be set to true.
  bool MatchViewOrStatic(const std::string& method, const std::string& url,
                         bool* stream);

  // Serve static files from the doc root.
  ResponsePtr ServeStatic(RequestPtr request);

private:
  // Port number.
  std::uint16_t port_;

  // The directory with the static files to be served.
  std::filesystem::path doc_root_;

  // The size of the chunk loaded into memory each time when serving a
  // static file.
  std::size_t file_chunk_size_;

  // Is the server running?
  bool running_;

  // The mutex for guarding the state of the server.
  std::mutex state_mutex_;

  // The io_context used to perform asynchronous operations.
  asio::io_context io_context_;

  // Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor acceptor_;

  // The connection pool which owns all live connections.
  ConnectionPool pool_;

  // The signals for processing termination notifications.
  asio::signal_set signals_;

  // Worker threads.
  std::vector<std::thread> worker_threads_;

  // The queue with connection waiting for the workers to process.
  Queue<ConnectionPtr> queue_;
};

}  // namespace webcc

#endif  // WEBCC_SERVER_H_
