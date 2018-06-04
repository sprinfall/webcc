#include "webcc/rest_async_client.h"

namespace webcc {

void RestAsyncClient::Request(const std::string& method,
                              const std::string& url,
                              const std::string& content,
                              HttpResponseHandler response_handler) {
  response_handler_ = response_handler;

  HttpRequestPtr request(new webcc::HttpRequest());

  request->set_method(method);
  request->set_url(url);
  request->SetHost(host_, port_);

  if (!content.empty()) {
    request->SetContent(content);
  }

  request->Build();

  HttpAsyncClientPtr http_client(new HttpAsyncClient(io_context_));
  http_client->Request(request, response_handler_);
}

}  // namespace webcc
