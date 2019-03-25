#ifndef WEBCC_HTTP_CONNECTION_POOL_H_
#define WEBCC_HTTP_CONNECTION_POOL_H_

#include <set>

#include "webcc/http_connection.h"

namespace webcc {

class HttpConnectionPool {
public:
  HttpConnectionPool(const HttpConnectionPool&) = delete;
  HttpConnectionPool& operator=(const HttpConnectionPool&) = delete;

  HttpConnectionPool();

  // Add a connection to the pool and start it.
  void Start(HttpConnectionPtr c);

  // Close a connection.
  void Close(HttpConnectionPtr c);

  // Close all connections.
  void CloseAll();

private:
  /// The managed connections.
  std::set<HttpConnectionPtr> connections_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CONNECTION_POOL_H_
