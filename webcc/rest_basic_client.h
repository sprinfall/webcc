#ifndef WEBCC_REST_BASIC_CLIENT_H_
#define WEBCC_REST_BASIC_CLIENT_H_

#include <cassert>
#include <string>
#include <utility>  // for move()

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

namespace webcc {

template <class HttpClientType>
class RestBasicClient {
 public:
  // If |port| is empty, |host| will be checked to see if it contains port or
  // not (separated by ':').
  explicit RestBasicClient(const std::string& host,
                           const std::string& port = "")
      : host_(host), port_(port) {
    if (port_.empty()) {
      std::size_t i = host_.find_last_of(':');
      if (i != std::string::npos) {
        port_ = host_.substr(i + 1);
        host_ = host_.substr(0, i);
      }
    }
  }

  ~RestBasicClient() = default;

  WEBCC_DELETE_COPY_ASSIGN(RestBasicClient);

  void SetTimeout(int seconds) {
    http_client_.SetTimeout(seconds);
  }

  // NOTE:
  // The return value of the following methods (Get, Post, etc.) only indicates
  // if the socket communication is successful or not. Check error() and
  // timed_out() for more information if it's failed. Check response_status()
  // instead for the HTTP status code.

  // HTTP GET request.
  inline bool Get(const std::string& url) {
    return Request(kHttpGet, url, "");
  }

  // HTTP POST request.
  inline bool Post(const std::string& url, std::string&& content) {
    return Request(kHttpPost, url, std::move(content));
  }

  // HTTP PUT request.
  inline bool Put(const std::string& url, std::string&& content) {
    return Request(kHttpPut, url, std::move(content));
  }

  // HTTP PATCH request.
  inline bool Patch(const std::string& url, std::string&& content) {
    return Request(kHttpPatch, url, std::move(content));
  }

  // HTTP DELETE request.
  inline bool Delete(const std::string& url) {
    return Request(kHttpDelete, url, "");
  }

  HttpResponsePtr response() const {
    return http_client_.response();
  }

  int response_status() const {
    assert(response());
    return response()->status();
  }

  const std::string& response_content() const {
    assert(response());
    return response()->content();
  }

  bool timed_out() const {
    return http_client_.timed_out();
  }

  Error error() const {
    return http_client_.error();
  }

 private:
  bool Request(const std::string& method, const std::string& url,
               std::string&& content) {
    HttpRequest http_request;

    http_request.set_method(method);
    http_request.set_url(url);
    http_request.set_host(host_, port_);

    if (!content.empty()) {
      http_request.SetContent(std::move(content), true);
      http_request.SetContentType(kAppJsonUtf8);
    }

    http_request.Make();

    if (!http_client_.Request(http_request)) {
      return false;
    }

    return true;
  }

  std::string host_;
  std::string port_;

  HttpClientType http_client_;
};

}  // namespace webcc

#endif  // WEBCC_REST_BASIC_CLIENT_H_
