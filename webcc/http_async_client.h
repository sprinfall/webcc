#ifndef WEBCC_HTTP_ASYNC_CLIENT_H_
#define WEBCC_HTTP_ASYNC_CLIENT_H_

#include "webcc/http_async_client_base.h"

namespace webcc {

class HttpAsyncClient;
typedef std::shared_ptr<HttpAsyncClient> HttpAsyncClientPtr;

// HTTP asynchronous client.
class HttpAsyncClient : public HttpAsyncClientBase {
 public:
  ~HttpAsyncClient() = default;

  // Forbid to create HttpAsyncClient in stack since it's derived from
  // std::shared_from_this.
  static HttpAsyncClientPtr New(boost::asio::io_context& io_context,
                                std::size_t buffer_size = 0) {
    return HttpAsyncClientPtr{
      new HttpAsyncClient(io_context, buffer_size)
    };
  }

 private:
  explicit HttpAsyncClient(boost::asio::io_context& io_context,
                           std::size_t buffer_size = 0);

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

}  // namespace webcc

#endif  // WEBCC_HTTP_ASYNC_CLIENT_H_
