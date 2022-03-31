#ifndef WEBCC_CLIENT_POOL_H_
#define WEBCC_CLIENT_POOL_H_

#include <map>
#include <string>

#include "webcc/client.h"
#include "webcc/url.h"

namespace webcc {

// Connection pool for persistent (keep-alive) connections.
class ClientPool {
public:
  struct Key {
    std::string scheme;
    std::string host;
    std::string port;

    Key() = default;

    explicit Key(const Url& url)
        : scheme(url.scheme()), host(url.host()), port(url.port()) {
    }

    bool operator==(const Key& rhs) const {
      return scheme == rhs.scheme && host == rhs.host && port == rhs.port;
    }

    bool operator<(const Key& rhs) const {
      if (scheme < rhs.scheme) {
        return true;
      }
      if (host < rhs.host) {
        return true;
      }
      if (port < rhs.port) {
        return true;
      }
      return false;
    }
  };

public:
  ClientPool() = default;

  ClientPool(const ClientPool&) = delete;
  ClientPool& operator=(const ClientPool&) = delete;

  ~ClientPool();

  ClientPtr Get(const Key& key) const;

  void Add(const Key& key, ClientPtr client);

  void Remove(const Key& key);

private:
  std::map<Key, ClientPtr> clients_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_POOL_H_
