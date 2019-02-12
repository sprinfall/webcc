#include "webcc/rest_ssl_client.h"
#include "webcc/utility.h"

namespace webcc {

RestSslClient::RestSslClient(const std::string& host, const std::string& port,
                             std::size_t buffer_size, bool ssl_verify)
    : host_(host), port_(port),
      http_client_(buffer_size, ssl_verify) {
  AdjustHostPort(host_, port_);
}

bool RestSslClient::Request(const std::string& method, const std::string& url,
                            std::string&& content, std::size_t buffer_size) {
  HttpRequest http_request(method, url, host_, port_);

  if (!content.empty()) {
    http_request.SetContent(std::move(content), true);
    http_request.SetContentType(http::media_types::kApplicationJson,
                                http::charsets::kUtf8);
  }

  http_request.Prepare();

  if (!http_client_.Request(http_request, buffer_size)) {
    return false;
  }

  return true;
}

}  // namespace webcc
