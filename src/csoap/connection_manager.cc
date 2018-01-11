#include "csoap/connection_manager.h"

namespace csoap {

ConnectionManager::ConnectionManager() {
}

void ConnectionManager::Start(ConnectionPtr conn) {
  connections_.insert(conn);
  conn->Start();
}

void ConnectionManager::Stop(ConnectionPtr conn) {
  connections_.erase(conn);
  conn->Stop();
}

void ConnectionManager::StopAll() {
  for (const ConnectionPtr& conn : connections_) {
    conn->Stop();
  }
  connections_.clear();
}

}  // namespace csoap
