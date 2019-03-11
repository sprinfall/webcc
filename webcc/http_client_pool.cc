#include "webcc/http_client_pool.h"

namespace webcc {

HttpClientPtr HttpClientPool::Get(const Url& url) const {
  for (const auto& client : clients_) {
    return client;
  }
  return HttpClientPtr{};
}

void HttpClientPool::Add(HttpClientPtr client) {
  clients_.push_back(client);
}

}  // namespace webcc
