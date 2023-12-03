#ifndef WEBCC_CLIENT_POOL_H_
#define WEBCC_CLIENT_POOL_H_

#include <cassert>
#include <map>
#include <mutex>
#include <string>

#include "webcc/client_base.h"

namespace webcc {

// A pool of persistent (keep-alive) client connections.
class ClientPool {
public:
  using Key = std::string;

  ClientPool() = default;

  ClientPool(const ClientPool&) = delete;
  ClientPool& operator=(const ClientPool&) = delete;

  ~ClientPool() {
    // NOTE: Clear() should be called manually!
    assert(IsEmpty());
  }

  ClientPtr Get(const Key& key) const;

  bool IsEmpty() const;

  void Add(const Key& key, ClientPtr client);

  void Remove(const Key& key);

  // Shut down and close all connections.
  void Clear(bool* new_async_op);

private:
  std::map<Key, ClientPtr> clients_;

  mutable std::mutex mutex_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_POOL_H_
