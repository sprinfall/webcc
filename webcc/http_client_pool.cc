#include "webcc/http_client_pool.h"

#include "webcc/logger.h"

namespace webcc {

HttpClientPool::~HttpClientPool() {
  if (!clients_.empty()) {
    LOG_INFO("Close socket for all (%u) connections in the pool.",
             clients_.size());

    for (auto& pair : clients_) {
      pair.second->Close();
    }
  }
}

HttpClientPtr HttpClientPool::Get(const Key& key) const {
  auto it = clients_.find(key);

  if (it != clients_.end()) {
    return it->second;
  } else {
    return HttpClientPtr{};
  }
}

void HttpClientPool::Add(const Key& key, HttpClientPtr client) {
  clients_[key] = client;

  LOG_INFO("Added connection to pool (%s, %s, %s).",
           key.scheme.c_str(), key.host.c_str(), key.port.c_str());
}

void HttpClientPool::Remove(const Key& key) {
  clients_.erase(key);

  LOG_INFO("Removed connection from pool (%s, %s, %s).",
           key.scheme.c_str(), key.host.c_str(), key.port.c_str());
}

}  // namespace webcc
