#include "webcc/connection_pool.h"

#include "webcc/logger.h"

namespace webcc {

void ConnectionPool::Start(ConnectionPtr c) {
  LOG_VERB("Starting connection...");
  connections_.insert(c);
  c->Start();
}

void ConnectionPool::Close(ConnectionPtr c) {
  LOG_VERB("Closing connection...");
  connections_.erase(c);
  c->Close();
}

void ConnectionPool::CloseAll() {
  LOG_VERB("Closing all (%u) connections...", connections_.size());
  for (auto& c : connections_) {
    c->Close();
  }
  connections_.clear();
}

}  // namespace webcc
