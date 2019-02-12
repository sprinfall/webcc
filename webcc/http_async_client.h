#ifndef WEBCC_HTTP_ASYNC_CLIENT_H_
#define WEBCC_HTTP_ASYNC_CLIENT_H_

#include "webcc/http_async_client_base.h"

namespace webcc {

// HTTP asynchronous client.
class HttpAsyncClient : public HttpAsyncClientBase {
 public:
  explicit HttpAsyncClient(boost::asio::io_context& io_context,
                           std::size_t buffer_size = 0);

  ~HttpAsyncClient() = default;

 private:
  void Resolve() final {
    DoResolve(kHttpPort);
  }

  void OnConnected() final {
    DoWrite();
  }

  void SocketAsyncConnect(const Endpoints& endpoints,
                          ConnectHandler&& handler) final;

  void SocketAsyncWrite(WriteHandler&& handler) final;

  void SocketAsyncReadSome(ReadHandler&& handler) final;

  void SocketClose(boost::system::error_code* ec) final;

  tcp::socket socket_;
};

typedef std::shared_ptr<HttpAsyncClient> HttpAsyncClientPtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_H_
