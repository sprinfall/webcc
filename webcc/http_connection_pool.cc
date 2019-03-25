#include "webcc/http_connection_pool.h"

#include "webcc/logger.h"

namespace webcc {

HttpConnectionPool::HttpConnectionPool() {
}

void HttpConnectionPool::Start(HttpConnectionPtr c) {
  LOG_VERB("Starting connection...");
  connections_.insert(c);
  c->Start();
}

void HttpConnectionPool::Close(HttpConnectionPtr c) {
  LOG_VERB("Closing connection...");
  connections_.erase(c);
  c->Close();
}

void HttpConnectionPool::CloseAll() {
  LOG_VERB("Closing all (%u) connections...", connections_.size());
  for (auto& c : connections_) {
    c->Close();
  }
  connections_.clear();
}

}  // namespace webcc
