#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include "webcc/http_client_base.h"

namespace webcc {

// HTTP synchronous client.
class HttpClient : public HttpClientBase {
 public:
  explicit HttpClient(std::size_t buffer_size = 0);

  ~HttpClient() = default;

 private:
  Error Connect(const HttpRequest& request) final {
    return DoConnect(request, kHttpPort);
  }

  void SocketConnect(const Endpoints& endpoints,
                     boost::system::error_code* ec) final;

  void SocketWrite(const HttpRequest& request,
                   boost::system::error_code* ec) final;

  void SocketAsyncReadSome(ReadHandler&& handler) final;

  void SocketClose(boost::system::error_code* ec) final;

  boost::asio::ip::tcp::socket socket_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
