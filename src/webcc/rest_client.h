#ifndef WEBCC_REST_CLIENT_H_
#define WEBCC_REST_CLIENT_H_

#include <cassert>
#include <string>

#include "webcc/globals.h"
#include "webcc/http_response.h"

namespace webcc {

class RestClient {
 public:
  RestClient(const std::string& host, const std::string& port)
      : host_(host), port_(port) {
  }

  void set_timeout_seconds(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
  }

  HttpResponsePtr response() const { return response_; }

  int response_status() const {
    assert(response_);
    return response_->status();
  }

  const std::string& response_content() const {
    assert(response_);
    return response_->content();
  }

  Error error() const { return error_; }

  bool timed_out() const { return timed_out_; }

  bool Get(const std::string& url) {
    return Request(kHttpGet, url, "");
  }

  bool Post(const std::string& url, const std::string& content) {
    return Request(kHttpPost, url, content);
  }

  bool Put(const std::string& url, const std::string& content) {
    return Request(kHttpPut, url, content);
  }

  bool Patch(const std::string& url, const std::string& content) {
    return Request(kHttpPatch, url, content);
  }

  bool Delete(const std::string& url) {
    return Request(kHttpDelete, url, "");
  }

 private:
  bool Request(const std::string& method,
               const std::string& url,
               const std::string& content);

  std::string host_;
  std::string port_;

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_ = 0;

  HttpResponsePtr response_;

  Error error_ = kNoError;

  // If the error was caused by timeout or not.
  bool timed_out_ = false;
};

}  // namespace webcc

#endif  // WEBCC_REST_CLIENT_H_
