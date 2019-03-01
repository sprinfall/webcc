#ifndef WEBCC_REST_SSL_CLIENT_H_
#define WEBCC_REST_SSL_CLIENT_H_

#include <cassert>
#include <map>
#include <string>
#include <utility>  // for move()

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/http_ssl_client.h"

namespace webcc {

typedef std::map<std::string, std::string> SSMap;

class RestClientBase {
public:
  RestClientBase(HttpClientBase* http_client_base,
                 const SSMap& headers = {});

  virtual ~RestClientBase() = default;

  WEBCC_DELETE_COPY_ASSIGN(RestClientBase);

  void SetTimeout(int seconds) {
    http_client_base_->SetTimeout(seconds);
  }

  // Overwrite the default content type (json & utf-8).
  void SetContentType(const std::string& media_type,
                      const std::string& charset) {
    content_media_type_ = media_type;
    content_charset_ = charset;
  }

  HttpResponsePtr response() const {
    return http_client_base_->response();
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
    return http_client_base_->timed_out();
  }

  Error error() const {
    return http_client_base_->error();
  }

protected:
  // Default: "application/json".
  std::string content_media_type_;
  // Default: "utf-8".
  std::string content_charset_;

  // Default headers for all session requests.
  std::map<std::string, std::string> headers_;

private:
  HttpClientBase* http_client_base_;
};


class RestSslClient : public RestClientBase {
public:
  // If |port| is empty, |host| will be checked to see if it contains port or
  // not (separated by ':').
  explicit RestSslClient(const std::string& host,
                         const std::string& port = "",
                         bool ssl_verify = true,
                         const SSMap& headers = {},
                         std::size_t buffer_size = 0);

  ~RestSslClient() = default;

  // NOTE:
  // The return value of the following methods (Get, Post, etc.) only indicates
  // if the socket communication is successful or not. Check error() and
  // timed_out() for more information if it's failed. Check response_status()
  // instead for the HTTP status code.

  inline bool Get(const std::string& url, const SSMap& headers = {},
                  std::size_t buffer_size = 0) {
    return Request(kHttpGet, url, "", headers, buffer_size);
  }

  inline bool Post(const std::string& url, std::string&& content,
                   const SSMap& headers = {}, std::size_t buffer_size = 0) {
    return Request(kHttpPost, url, std::move(content), headers, buffer_size);
  }

  inline bool Put(const std::string& url, std::string&& content,
                  const SSMap& headers = {}, std::size_t buffer_size = 0) {
    return Request(kHttpPut, url, std::move(content), headers, buffer_size);
  }

  inline bool Patch(const std::string& url, std::string&& content,
                    const SSMap& headers = {}, std::size_t buffer_size = 0) {
    return Request(kHttpPatch, url, std::move(content), headers, buffer_size);
  }

  inline bool Delete(const std::string& url, const SSMap& headers = {},
                     std::size_t buffer_size = 0) {
    return Request(kHttpDelete, url, "", headers, buffer_size);
  }

  bool Request(const std::string& method, const std::string& url,
               std::string&& content, const SSMap& headers = {},
               std::size_t buffer_size = 0);

 private:
  std::string host_;
  std::string port_;

  HttpSslClient http_ssl_client_;
};

}  // namespace webcc

#endif  // WEBCC_REST_SSL_CLIENT_H_
