#ifndef WEBCC_CONNECTION_POOL_H_
#define WEBCC_CONNECTION_POOL_H_

#include <set>

#include "webcc/connection.h"

namespace webcc {

class ConnectionPool {
public:
  ConnectionPool() = default;

  ConnectionPool(const ConnectionPool&) = delete;
  ConnectionPool& operator=(const ConnectionPool&) = delete;

  // Add a connection to the pool and start it.
  void Start(ConnectionPtr c);

  // Close a connection.
  void Close(ConnectionPtr c);

  // Close all pending connections.
  void CloseAll();

private:
  std::set<ConnectionPtr> connections_;
};

}  // namespace webcc

#endif  // WEBCC_CONNECTION_POOL_H_
