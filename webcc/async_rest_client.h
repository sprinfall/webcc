#ifndef WEBCC_ASYNC_REST_CLIENT_H_
#define WEBCC_ASYNC_REST_CLIENT_H_

#include <string>
#include <utility>  // for move()

#include "webcc/async_http_client.h"

namespace webcc {

class AsyncRestClient {
 public:
  AsyncRestClient(boost::asio::io_context& io_context,  // NOLINT
                  const std::string& host, const std::string& port);

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  void Get(const std::string& url,
           HttpResponseHandler response_handler) {
    Request(kHttpGet, url, "", response_handler);
  }

  void Post(const std::string& url, std::string&& content,
            HttpResponseHandler response_handler) {
    Request(kHttpPost, url, std::move(content), response_handler);
  }

  void Put(const std::string& url, std::string&& content,
           HttpResponseHandler response_handler) {
    Request(kHttpPut, url, std::move(content), response_handler);
  }

  void Patch(const std::string& url, std::string&& content,
             HttpResponseHandler response_handler) {
    Request(kHttpPatch, url, std::move(content), response_handler);
  }

  void Delete(const std::string& url,
              HttpResponseHandler response_handler) {
    Request(kHttpDelete, url, "", response_handler);
  }

 private:
  void Request(const std::string& method, const std::string& url,
               std::string&& content, HttpResponseHandler response_handler);

  boost::asio::io_context& io_context_;

  std::string host_;
  std::string port_;

  HttpResponseHandler response_handler_;

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_;
};

}  // namespace webcc

#endif  // WEBCC_ASYNC_REST_CLIENT_H_
