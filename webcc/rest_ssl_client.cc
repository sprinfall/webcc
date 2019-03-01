#include "webcc/rest_ssl_client.h"

#include "boost/algorithm/string.hpp"

#include "webcc/utility.h"

namespace webcc {

RestClientBase::RestClientBase(HttpClientBase* http_client_base,
                               const SSMap& headers)
    : http_client_base_(http_client_base),
      content_media_type_(http::media_types::kApplicationJson),
      content_charset_(http::charsets::kUtf8),
      headers_(headers) {
}


RestSslClient::RestSslClient(const std::string& host, const std::string& port,
                             bool ssl_verify, const SSMap& headers,
                             std::size_t buffer_size)
    : RestClientBase(&http_ssl_client_, headers),
      host_(host), port_(port),
      http_ssl_client_(ssl_verify, buffer_size) {
  AdjustHostPort(host_, port_);
}

bool RestSslClient::Request(const std::string& method, const std::string& url,
                            std::string&& content, const SSMap& headers,
                            std::size_t buffer_size) {
  HttpRequest http_request(method, url, host_, port_);

  if (!content.empty()) {
    http_request.SetContent(std::move(content), true);
    http_request.SetContentType(content_media_type_, content_charset_);
  }

  for (auto& h : headers_) {
    http_request.SetHeader(h.first, h.second);
  }

  for (auto& h : headers) {
    http_request.SetHeader(h.first, h.second);
  }

  http_request.Prepare();

  if (!http_ssl_client_.Request(http_request, buffer_size)) {
    return false;
  }

  return true;
}

}  // namespace webcc
