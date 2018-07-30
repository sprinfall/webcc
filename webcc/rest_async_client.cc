#include "webcc/rest_async_client.h"

namespace webcc {

RestAsyncClient::RestAsyncClient(boost::asio::io_context& io_context,
                                 const std::string& host,
                                 const std::string& port)
    : io_context_(io_context), host_(host), port_(port), timeout_seconds_(0) {
}

void RestAsyncClient::Request(const std::string& method,
                              const std::string& url,
                              std::string&& content,
                              HttpResponseHandler response_handler) {
  response_handler_ = response_handler;

  HttpRequestPtr request(new webcc::HttpRequest());

  request->set_method(method);
  request->set_url(url);
  request->SetHost(host_, port_);

  if (!content.empty()) {
    request->SetContent(std::move(content), true);
    request->SetContentType(kAppJsonUtf8);
  }

  request->UpdateStartLine();

  HttpAsyncClientPtr http_client(new HttpAsyncClient(io_context_));

  if (timeout_seconds_ > 0) {
    http_client->set_timeout_seconds(timeout_seconds_);
  }

  http_client->Request(request, response_handler_);
}

}  // namespace webcc
