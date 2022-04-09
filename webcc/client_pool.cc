#include "webcc/client_pool.h"

#include "webcc/logger.h"

namespace webcc {

BlockingClientPtr ClientPool::Get(const Key& key) const {
  std::lock_guard<std::mutex> lock{ mutex_ };

  auto it = clients_.find(key);
  if (it != clients_.end()) {
    return it->second;
  } else {
    return {};
  }
}

void ClientPool::Add(const Key& key, BlockingClientPtr client) {
  std::lock_guard<std::mutex> lock{ mutex_ };

  clients_[key] = client;
  LOG_INFO("Connection added to pool (%s)", key.c_str());
}

void ClientPool::Remove(const Key& key) {
  std::lock_guard<std::mutex> lock{ mutex_ };

  if (clients_.erase(key) == 1) {
    LOG_INFO("Connection removed from pool (%s)", key.c_str());
  }
}

void ClientPool::Clear() {
  std::lock_guard<std::mutex> lock{ mutex_ };

  if (!clients_.empty()) {
    LOG_INFO("Close socket for all (%u) connections in the pool",
             clients_.size());
    for (auto& pair : clients_) {
      pair.second->Close();
    }
    clients_.clear();
  }
}

}  // namespace webcc
