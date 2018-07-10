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

void RestRequestHandler::HandleConnection(HttpConnectionPtr connection) {
  Url url(connection->request().url(), true);

  if (!url.IsValid()) {
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  std::vector<std::string> sub_matches;
  RestServicePtr service = service_manager_.GetService(url.path(),
                                                       &sub_matches);
  if (!service) {
    LOG_WARN("No service matches the URL: %s", url.path().c_str());
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  UrlQuery query;
  Url::SplitQuery(url.query(), &query);

  std::string content;
  bool ok = service->Handle(connection->request().method(),
                            sub_matches,
                            query,
                            connection->request().content(),
                            &content);
  if (!ok) {
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  if (!content.empty()) {
    connection->SetResponseContent(std::move(content), kAppJsonUtf8);
  }
  connection->SendResponse(HttpStatus::kOK);
}

}  // namespace webcc
