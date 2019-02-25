#include "webcc/rest_async_client.h"

namespace webcc {

RestAsyncClient::RestAsyncClient(boost::asio::io_context& io_context,
                                 const std::string& host,
                                 const std::string& port,
                                 std::size_t buffer_size)
    : io_context_(io_context),
      host_(host), port_(port),
      timeout_seconds_(0),
      buffer_size_(buffer_size) {
}

void RestAsyncClient::Request(const std::string& method,
                              const std::string& url,
                              std::string&& content,
                              HttpResponseCallback callback) {
  HttpRequestPtr http_request(new HttpRequest(method, url, host_, port_));

  if (!content.empty()) {
    http_request->SetContent(std::move(content), true);
    http_request->SetContentType(http::media_types::kApplicationJson,
                                 http::charsets::kUtf8);
  }

  http_request->Prepare();

  auto http_async_client = HttpAsyncClient::New(io_context_, buffer_size_);

  if (timeout_seconds_ > 0) {
    http_async_client->SetTimeout(timeout_seconds_);
  }

  http_async_client->Request(http_request, callback);
}

}  // namespace webcc
