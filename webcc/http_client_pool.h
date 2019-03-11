#ifndef WEBCC_HTTP_CLIENT_POOL_H_
#define WEBCC_HTTP_CLIENT_POOL_H_

// HTTP client connection pool for keep-alive connections.

#include <list>
#include <memory>
#include <string>

#include "webcc/http_client.h"
#include "webcc/url.h"

namespace webcc {

typedef std::shared_ptr<HttpClient> HttpClientPtr;

class HttpClientPool {
public:
  HttpClientPool() = default;

  HttpClientPtr Get(const Url& url) const;

  void Add(HttpClientPtr client);

private:
  std::list<HttpClientPtr> clients_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_POOL_H_
