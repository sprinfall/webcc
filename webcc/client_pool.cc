#include "webcc/client_pool.h"

#include "webcc/logger.h"

namespace webcc {

ClientPtr ClientPool::Get(const Key& key) const {
  std::lock_guard<std::mutex> lock{ mutex_ };

  auto iter = clients_.find(key);
  if (iter != clients_.end()) {
    return iter->second;
  } else {
    return {};
  }
}

bool ClientPool::IsEmpty() const {
  std::lock_guard<std::mutex> lock{ mutex_ };

  return clients_.empty();
}

void ClientPool::Add(const Key& key, ClientPtr client) {
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

void ClientPool::Clear(bool* new_async_op) {
  std::lock_guard<std::mutex> lock{ mutex_ };

  if (clients_.empty()) {
    return;
  }

  LOG_INFO("Close socket for all (%u) connections in the pool",
           clients_.size());

  for (auto& pair : clients_) {
    if (pair.second->Close()) {
      *new_async_op = true;
    }
  }

  clients_.clear();
}

}  // namespace webcc
