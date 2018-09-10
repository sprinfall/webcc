#include "webcc/rest_request_handler.h"

#include <utility>  // for move()
#include <vector>

#include "webcc/logger.h"
#include "webcc/url.h"

namespace webcc {

bool RestRequestHandler::Bind(RestServicePtr service, const std::string& url,
                              bool is_regex) {
  return service_manager_.AddService(service, url, is_regex);
}

void RestRequestHandler::HandleSession(HttpSessionPtr session) {
  const HttpRequest& http_request = session->request();

  Url url(http_request.url(), /*decode*/true);

  if (!url.IsValid()) {
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  RestRequest rest_request{
    http_request.method(), http_request.content(), url.query()
  };

  // Get service by URL path.
  RestServicePtr service =
      service_manager_.GetService(url.path(), &rest_request.url_sub_matches);

  if (!service) {
    LOG_WARN("No service matches the URL path: %s", url.path().c_str());
    session->SendResponse(HttpStatus::kNotFound);
    return;
  }

  RestResponse rest_response;
  service->Handle(rest_request, &rest_response);

  if (!rest_response.content.empty()) {
    session->SetResponseContent(std::move(rest_response.content),
                                kAppJsonUtf8);
  }

  // Send response back to client.
  session->SendResponse(rest_response.status);
}

}  // namespace webcc
