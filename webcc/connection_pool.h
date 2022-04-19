#ifndef WEBCC_CONNECTION_POOL_H_
#define WEBCC_CONNECTION_POOL_H_

#include <mutex>
#include <set>

#include "webcc/connection.h"

namespace webcc {

class ConnectionPool {
public:
  ConnectionPool() = default;

  ConnectionPool(const ConnectionPool&) = delete;
  ConnectionPool& operator=(const ConnectionPool&) = delete;

  // Add the connection and start to read the request from it.
  // Called when a new connection has just been accepted.
  void Start(ConnectionPtr connection);

  // Close the connection.
  // Called when the response of the connection has been sent back.
  void Close(ConnectionPtr connection);

  // Close all pending connections.
  // Called when the server is about to stop.
  void Clear();

private:
  std::set<ConnectionPtr> connections_;

  // Mutex is necessary if the loop is running in multiple threads.
  // See Server::Run().
  std::mutex mutex_;
};

}  // namespace webcc

#endif  // WEBCC_CONNECTION_POOL_H_
