#include "webcc/connection_pool.h"

#include "webcc/logger.h"

namespace webcc {

void ConnectionPool::Start(ConnectionPtr c) {
  LOG_VERB("Start connection");

  {
    // Lock the container only.
    std::lock_guard<std::mutex> lock{ mutex_ };
    connections_.insert(c);
  }

  c->Start();
}

void ConnectionPool::Close(ConnectionPtr c) {
  LOG_VERB("Close connection");

  {
    // Lock the container only.
    std::lock_guard<std::mutex> lock{ mutex_ };
    connections_.erase(c);
  }

  c->Close();
}

void ConnectionPool::Clear() {
  // Lock all since we are going to stop anyway.
  std::lock_guard<std::mutex> lock{ mutex_ };

  if (!connections_.empty()) {
    LOG_VERB("Close all (%u) connections", connections_.size());
    for (auto& c : connections_) {
      c->Close();
    }
    connections_.clear();
  }
}

}  // namespace webcc
