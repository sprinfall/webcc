#ifndef WEBCC_HTTP_SSL_ASYNC_CLIENT_H_
#define WEBCC_HTTP_SSL_ASYNC_CLIENT_H_

#include "webcc/http_async_client_base.h"

#include "boost/asio/ssl.hpp"

namespace webcc {

// HTTP SSL (a.k.a., HTTPS) asynchronous client.
class HttpSslAsyncClient : public HttpAsyncClientBase {
 public:
  // SSL verification (|ssl_verify|) needs CA certificates to be found
  // in the default verify paths of OpenSSL. On Windows, it means you need to
  // set environment variable SSL_CERT_FILE properly.
  explicit HttpSslAsyncClient(boost::asio::io_context& io_context,
                              std::size_t buffer_size = 0,
                              bool ssl_verify = true);

  ~HttpSslAsyncClient() = default;

  // See https://stackoverflow.com/q/657155/6825348
  std::shared_ptr<HttpSslAsyncClient> shared_from_this() {
    return shared_from_base<HttpSslAsyncClient>();
  }

 private:
  void Resolve() final;

  // Override to do handshake after connected.
  void OnConnected() final {
    DoHandshake();
  }

  void DoHandshake();
  void OnHandshake(boost::system::error_code ec);

  void SocketAsyncConnect(const Endpoints& endpoints,
                          ConnectHandler&& handler) final;

  void SocketAsyncWrite(WriteHandler&& handler) final;

  void SocketAsyncReadSome(ReadHandler&& handler) final;

  void SocketClose(boost::system::error_code* ec) final;

  boost::asio::ssl::context ssl_context_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

  // Verify the certificate of the peer (remote server) or not.
  bool ssl_verify_;
};

typedef std::shared_ptr<HttpSslAsyncClient> HttpSslAsyncClientPtr;

}  // namespace webcc

#endif  // WEBCC_HTTP_SSL_ASYNC_CLIENT_H_
