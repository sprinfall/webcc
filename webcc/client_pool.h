#ifndef WEBCC_CLIENT_POOL_H_
#define WEBCC_CLIENT_POOL_H_

#include <map>
#include <mutex>
#include <string>

#include "webcc/client.h"

namespace webcc {

// A pool of persistent (keep-alive) client connections.
class ClientPool {
public:
  using Key = std::string;

  ClientPool() = default;

  ClientPool(const ClientPool&) = delete;
  ClientPool& operator=(const ClientPool&) = delete;

  ~ClientPool() {
    Clear();
  }

  BlockingClientPtr Get(const Key& key) const;

  void Add(const Key& key, BlockingClientPtr client);

  void Remove(const Key& key);

  // Shutdown and close all connections.
  void Clear();

private:
  std::map<Key, BlockingClientPtr> clients_;

  mutable std::mutex mutex_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_POOL_H_
