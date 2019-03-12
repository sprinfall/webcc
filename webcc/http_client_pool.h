#ifndef WEBCC_HTTP_CLIENT_POOL_H_
#define WEBCC_HTTP_CLIENT_POOL_H_

#include <map>
#include <string>

#include "webcc/http_client.h"
#include "webcc/url.h"

namespace webcc {

// Connection pool for keep-alive connections.
class HttpClientPool {
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
  HttpClientPool() = default;

  ~HttpClientPool();

  HttpClientPtr Get(const Key& key) const;

  void Add(const Key& key, HttpClientPtr client);

  void Remove(const Key& key);

private:
  std::map<Key, HttpClientPtr> clients_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_POOL_H_
