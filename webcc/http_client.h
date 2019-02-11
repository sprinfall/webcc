#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include "webcc/http_client_base.h"

namespace webcc {

// HTTP client in synchronous mode.
// A request will not return until the response is received or timeout occurs.
// Don't use the same HttpClient object in multiple threads.
class HttpClient : public HttpClientBase {
 public:
  explicit HttpClient(std::size_t buffer_size = 0);

  ~HttpClient() = default;

  WEBCC_DELETE_COPY_ASSIGN(HttpClient);

 private:
  Error Connect(const HttpRequest& request) final {
    return DoConnect(request, kHttpPort);
  }

  void SocketConnect(const Endpoints& endpoints,
                     boost::system::error_code* ec) final;

  void SocketWrite(const HttpRequest& request,
                   boost::system::error_code* ec) final;

  void SocketAsyncReadSome(std::vector<char>& buffer,
                           ReadHandler handler) final;

  void SocketClose(boost::system::error_code* ec) final;

  boost::asio::ip::tcp::socket socket_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
