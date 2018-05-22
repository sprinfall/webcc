#include "webcc/rest_client.h"

#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

namespace webcc {

bool RestClient::Request(const std::string& method,
                         const std::string& url,
                         const std::string& content,
                         HttpResponse* response) {
  HttpRequest request;

  request.set_method(method);
  request.set_url(url);
  request.SetHost(host_, port_);

  if (!content.empty()) {
    request.SetContent(content);
  }

  request.Build();

  HttpClient http_client;
  Error error = http_client.Request(request, response);

  return error == kNoError;
}

}  // namespace webcc
