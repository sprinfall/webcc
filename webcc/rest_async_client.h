#ifndef WEBCC_REST_ASYNC_CLIENT_H_
#define WEBCC_REST_ASYNC_CLIENT_H_

#include <string>
#include <utility>  // for move()

#include "webcc/http_async_client.h"

namespace webcc {

class RestAsyncClient {
 public:
  RestAsyncClient(boost::asio::io_context& io_context,
                  const std::string& host, const std::string& port,
                  std::size_t buffer_size = 0);

  void SetTimeout(int seconds) {
    timeout_seconds_ = seconds;
  }

  void Get(const std::string& url, HttpResponseCallback callback) {
    Request(kHttpGet, url, "", callback);
  }

  void Post(const std::string& url, std::string&& content,
            HttpResponseCallback callback) {
    Request(kHttpPost, url, std::move(content), callback);
  }

  void Put(const std::string& url, std::string&& content,
           HttpResponseCallback callback) {
    Request(kHttpPut, url, std::move(content), callback);
  }

  void Patch(const std::string& url, std::string&& content,
             HttpResponseCallback callback) {
    Request(kHttpPatch, url, std::move(content), callback);
  }

  void Delete(const std::string& url, HttpResponseCallback callback) {
    Request(kHttpDelete, url, "", callback);
  }

 private:
  void Request(const std::string& method, const std::string& url,
               std::string&& content, HttpResponseCallback callback);

  boost::asio::io_context& io_context_;

  std::string host_;
  std::string port_;

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_;

  std::size_t buffer_size_;
};

}  // namespace webcc

#endif  // WEBCC_REST_ASYNC_CLIENT_H_
