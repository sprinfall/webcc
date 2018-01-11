#ifndef CSOAP_CONNECTION_MANAGER_H_
#define CSOAP_CONNECTION_MANAGER_H_

#include <set>
#include "csoap/connection.h"

namespace csoap {

// ConnectionManager manages open connections so that they may be cleanly
// stopped when the server needs to shut down.
class ConnectionManager {
public:
  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;

  // Construct a connection manager.
  ConnectionManager();

  // Add the specified connection to the manager and start it.
  void Start(ConnectionPtr conn);

  // Stop the specified connection.
  void Stop(ConnectionPtr conn);

  // Stop all connections.
  void StopAll();

private:
  // The managed connections.
  std::set<ConnectionPtr> connections_;
};

}  // namespace csoap

#endif  // CSOAP_CONNECTION_MANAGER_H_
