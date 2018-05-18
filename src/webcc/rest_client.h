#ifndef WEBCC_REST_CLIENT_H_
#define WEBCC_REST_CLIENT_H_

#include <string>

#include "webcc/globals.h"

namespace webcc {

class HttpResponse;

class RestClient {
public:
  RestClient(const std::string& host, const std::string& port)
      : host_(host), port_(port) {
  }

  bool Get(const std::string& url, HttpResponse* response) {
    return Request(kHttpGet, url, "", response);
  }

  bool Post(const std::string& url,
            const std::string& content,
            HttpResponse* response) {
    return Request(kHttpPost, url, content, response);
  }

  bool Put(const std::string& url,
           const std::string& content,
           HttpResponse* response) {
    return Request(kHttpPut, url, content, response);
  }

  bool Patch(const std::string& url,
             const std::string& content,
             HttpResponse* response) {
    return Request(kHttpPatch, url, content, response);
  }

  bool Delete(const std::string& url, HttpResponse* response) {
    return Request(kHttpDelete, url, "", response);
  }

private:
  bool Request(const std::string& method,
               const std::string& url,
               const std::string& content,
               HttpResponse* response);

  std::string host_;
  std::string port_;
};

}  // namespace webcc

#endif  // WEBCC_REST_CLIENT_H_
