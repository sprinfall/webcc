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
#include "webcc/router.h"
#include "webcc/url.h"

namespace webcc {

class Server : public Router {
public:
  Server(boost::asio::ip::tcp protocol, std::uint16_t port,
         const sfs::path& doc_root = {});

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  ~Server() override = default;

  void set_buffer_size(std::size_t buffer_size) {
    if (buffer_size > 0) {
      buffer_size_ = buffer_size;
    }
  }

  void set_file_chunk_size(std::size_t file_chunk_size) {
    assert(file_chunk_size > 0);
    file_chunk_size_ = file_chunk_size;
  }

  // Start and run the server.
  // This method is blocking so will not return until Stop() is called (from
  // another thread) or a signal like SIGINT is caught.
  // When the request of a connection has been read, the connection is put into
  // a queue waiting for some worker thread to process. Normally, the more
  // `workers` you have, the more concurrency you gain (the concurrency also
  // depends on the number of CPU cores). The worker thread pops connections
  // from the queue one by one, prepares the response by the user provided View,
  // then sends it back to the client.
  // Meanwhile, the (event) loop, i.e., io_context, is also running in a number
  // (`loops`) of threads. Normally, one thread for the loop is good enough, but
  // it could be more than that.
  void Run(std::size_t workers = 1, std::size_t loops = 1);

  // Stop the server.
  // This should be called from another thread since the Run() is blocking.
  void Stop();

  // Is the server running?
  bool IsRunning() const;

protected:
  // Create a new connection.
  virtual ConnectionPtr NewConnection();

  // Check if doc root is valid.
  // Absolute it if necessary.
  void CheckDocRoot();

  // Register signals which indicate when the server should exit.
  void AddSignals();

  // Wait for a signal to stop the server.
  void AsyncWaitSignals();

  // Listen on the given port.
  bool Listen(std::uint16_t port);

  // Accept connections asynchronously.
  void AsyncAccept();
  void OnAccept(ConnectionPtr connection, boost::system::error_code ec);

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

  // Serve static files from the doc root.
  ResponsePtr ServeStatic(RequestPtr request);

  // Translate a /-separated URL path to the local (relative) path.
  // Examples:
  //   (Non-Windows)
  //   "/path/to/file" -> "path/to/file"
  //   "/path/./to/../file" -> "path/to/file" (. and .. are ignored)
  //   "/path//to//file" -> "path/to/file"
  //   (Windows)
  //   "/path/to/file" -> "path\to\file"
  //   "/path\\sub/to/file" -> "to\file" (path\\sub is ignored)
  //   "/C:\\test/path" -> "path" (C:\\test is ignored)
  // Reference: Python http/server.py translate_path()
  sfs::path TranslatePath(const std::string& utf8_url_path);

protected:
  // tcp::v4() or tcp::v6()
  boost::asio::ip::tcp protocol_;

  // Port number.
  std::uint16_t port_ = 0;

  // The directory with the static files to be served.
  sfs::path doc_root_;

  // The size of the buffer for reading request.
  std::size_t buffer_size_ = kBufferSize;

  // The size of the chunk for serving static files.
  std::size_t file_chunk_size_ = 1024;

  // Is the server running?
  bool running_ = false;

  // The mutex for guarding the state of the server.
  std::mutex state_mutex_;

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

  // The queue with connection waiting for the workers to process.
  Queue<ConnectionPtr> queue_;
};

}  // namespace webcc

#endif  // WEBCC_SERVER_H_
