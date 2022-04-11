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
  // NOTE:
  // The connection might have already been closed by Clear().

  std::lock_guard<std::mutex> lock{ mutex_ };

  // Check the return value of erase() to see if it still exists or not.
  if (connections_.erase(c) == 1) {
    LOG_VERB("Close connection");
    c->Close();
  }  // else: Already closed by Clear()
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
