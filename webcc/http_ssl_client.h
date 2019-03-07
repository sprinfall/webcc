#ifndef WEBCC_HTTP_SSL_CLIENT_H_
#define WEBCC_HTTP_SSL_CLIENT_H_

#include "webcc/http_client_base.h"

#include "boost/asio/ssl.hpp"

namespace webcc {

// HTTP SSL (a.k.a., HTTPS) synchronous client.
class HttpSslClient : public HttpClientBase {
public:
  // SSL verification (|ssl_verify|) needs CA certificates to be found
  // in the default verify paths of OpenSSL. On Windows, it means you need to
  // set environment variable SSL_CERT_FILE properly.
  explicit HttpSslClient(bool ssl_verify = true, std::size_t buffer_size = 0);

  ~HttpSslClient() = default;

private:
  Error Handshake(const std::string& host);

  // Override to do handshake after connected.
  Error Connect(const HttpRequest& request) final;

  void SocketConnect(const Endpoints& endpoints,
                     boost::system::error_code* ec) final;

  void SocketWrite(const HttpRequest& request,
                   boost::system::error_code* ec) final;

  void SocketAsyncReadSome(ReadHandler&& handler) final;

  void SocketClose(boost::system::error_code* ec) final;

  boost::asio::ssl::context ssl_context_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

  // Verify the certificate of the peer (remote server) or not.
  bool ssl_verify_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_SSL_CLIENT_H_
