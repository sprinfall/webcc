#ifndef WEBCC_REST_ASYNC_CLIENT_H_
#define WEBCC_REST_ASYNC_CLIENT_H_

#include <string>

#include "webcc/globals.h"
#include "webcc/http_async_client.h"

namespace webcc {

class RestAsyncClient {
 public:
  RestAsyncClient(boost::asio::io_context& io_context,
                   const std::string& host, const std::string& port)
      : io_context_(io_context), host_(host), port_(port) {
  }

  void Get(const std::string& url,
           HttpResponseHandler response_handler) {
    Request(kHttpGet, url, "", response_handler);
  }

  void Post(const std::string& url,
            const std::string& content,
            HttpResponseHandler response_handler) {
    Request(kHttpPost, url, content, response_handler);
  }

  void Put(const std::string& url,
           const std::string& content,
           HttpResponseHandler response_handler) {
    Request(kHttpPut, url, content, response_handler);
  }

  void Patch(const std::string& url,
             const std::string& content,
             HttpResponseHandler response_handler) {
    Request(kHttpPatch, url, content, response_handler);
  }

  void Delete(const std::string& url,
              HttpResponseHandler response_handler) {
    Request(kHttpDelete, url, "", response_handler);
  }

 private:
  void Request(const std::string& method,
               const std::string& url,
               const std::string& content,
               HttpResponseHandler response_handler);

  boost::asio::io_context& io_context_;
  std::string host_;
  std::string port_;
  HttpResponseHandler response_handler_;
};

}  // namespace webcc

#endif  // WEBCC_REST_ASYNC_CLIENT_H_
