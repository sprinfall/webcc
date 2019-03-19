#ifndef WEBCC_HTTP_CLIENT_SESSION_H_
#define WEBCC_HTTP_CLIENT_SESSION_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/optional.hpp"

#include "webcc/http_client_pool.h"
#include "webcc/http_request_args.h"
#include "webcc/http_response.h"

namespace webcc {

class HttpClientSession {
public:
  HttpClientSession();

  ~HttpClientSession() = default;

  void set_content_type(const std::string& content_type) {
    content_type_ = content_type;
  }

  void set_charset(const std::string& charset) {
    charset_ = charset;
  }

  void AddHeader(const std::string& key, const std::string& value) {
    headers_.Add(key, value);
  }

  void set_ssl_verify(bool ssl_verify) {
    ssl_verify_.emplace(ssl_verify);
  }

  void set_buffer_size(std::size_t buffer_size) {
    buffer_size_ = buffer_size;
  }

  HttpResponsePtr Request(HttpRequestArgs&& args);

  HttpResponsePtr Get(const std::string& url,
                      std::vector<std::string>&& parameters = {},
                      std::vector<std::string>&& headers = {},
                      HttpRequestArgs&& args = HttpRequestArgs());

  HttpResponsePtr Post(const std::string& url, std::string&& data, bool json,
                       std::vector<std::string>&& headers = {},
                       HttpRequestArgs&& args = HttpRequestArgs());

  HttpResponsePtr Put(const std::string& url, std::string&& data, bool json,
                      std::vector<std::string>&& headers = {},
                      HttpRequestArgs&& args = HttpRequestArgs());

  HttpResponsePtr Delete(const std::string& url,
                         std::vector<std::string>&& headers = {},
                         HttpRequestArgs&& args = HttpRequestArgs());

private:
  void InitHeaders();

private:
  // E.g., "application/json".
  std::string content_type_;

  // E.g., "utf-8".
  std::string charset_;

  // Headers for each request sent from this session.
  HttpHeaderDict headers_;

  // Verify the certificate of the peer or not.
  boost::optional<bool> ssl_verify_;

  // The bytes of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_ = 0;

  // Connection pool for keep-alive.
  HttpClientPool pool_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_SESSION_H_
