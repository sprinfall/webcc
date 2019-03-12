#include "webcc/http_client_pool.h"

#include "webcc/logger.h"

namespace webcc {

HttpClientPool::~HttpClientPool() {
  LOG_INFO("Close socket for all (%u) connections in the pool.",
           clients_.size());

  for (auto& pair : clients_) {
    pair.second->Close();
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
}

void HttpClientPool::Remove(const Key& key) {
  clients_.erase(key);
}

}  // namespace webcc
