#include "webcc/rest_request_handler.h"

#include <utility>  // for move()
#include <vector>

#include "webcc/logger.h"
#include "webcc/url.h"

namespace webcc {

bool RestRequestHandler::Bind(RestServicePtr service,
                              const std::string& url,
                              bool is_regex) {
  return service_manager_.AddService(service, url, is_regex);
}

void RestRequestHandler::HandleSession(HttpSessionPtr session) {
  Url url(session->request().url(), true);

  if (!url.IsValid()) {
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  std::vector<std::string> sub_matches;
  RestServicePtr service = service_manager_.GetService(url.path(),
                                                       &sub_matches);
  if (!service) {
    LOG_WARN("No service matches the URL: %s", url.path().c_str());
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  UrlQuery query;
  Url::SplitQuery(url.query(), &query);

  std::string content;
  bool ok = service->Handle(session->request().method(),
                            sub_matches,
                            query,
                            session->request().content(),
                            &content);
  if (!ok) {
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  session->SetResponseContent(std::move(content), kTextJsonUtf8);
  session->SendResponse(HttpStatus::kOK);
}

}  // namespace webcc
