#include "webcc/rest_server.h"

#include "webcc/logger.h"
#include "webcc/url.h"

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

bool RestServiceManager::AddService(RestServicePtr service,
                                    const std::string& url) {
  assert(service);

  ServiceItem item(service, url);

  std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

  try {
    // Compile the regex.
    item.url_regex.assign(url, flags);

    service_items_.push_back(item);

    return true;

  } catch (std::regex_error& e) {
    LOG_ERRO("URL is not a valid regular expression: %s", e.what());
  }

  return false;
}

RestServicePtr RestServiceManager::GetService(
    const std::string& url,
    std::vector<std::string>* sub_matches) {
  assert(sub_matches != NULL);

  for (ServiceItem& item : service_items_) {
    std::smatch match;

    if (std::regex_match(url, match, item.url_regex)) {
      // Any sub-matches?
      // NOTE: Start from 1 because match[0] is the whole string itself.
      for (size_t i = 1; i < match.size(); ++i) {
        sub_matches->push_back(match[i].str());
      }

      return item.service;
    }
  }

  return RestServicePtr();
}

////////////////////////////////////////////////////////////////////////////////

bool RestRequestHandler::RegisterService(RestServicePtr service,
                                         const std::string& url) {
  return service_manager_.AddService(service, url);
}

void RestRequestHandler::HandleSession(HttpSessionPtr session) {
  Url url(session->request().url());

  if (!url.IsValid()) {
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  std::vector<std::string> sub_matches;
  RestServicePtr service = service_manager_.GetService(url.path(), &sub_matches);
  if (!service) {
    LOG_WARN("No service matches the URL: %s", url.path().c_str());
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  // TODO: Only for GET?
  Url::Query query = Url::SplitQuery(url.query());

  std::string content;
  bool ok = service->Handle(session->request().method(),
                            sub_matches,
                            query,
                            session->request().content(),
                            &content);
  if (!ok) {
    // TODO: Could be other than kBadRequest.
    session->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  session->SetResponseContent(std::move(content), kTextJsonUtf8);
  session->SendResponse(HttpStatus::kOK);
}

////////////////////////////////////////////////////////////////////////////////

RestServer::RestServer(unsigned short port, std::size_t workers)
    : HttpServer(port, workers)
    , request_handler_(new RestRequestHandler()) {
}

RestServer::~RestServer() {
  delete request_handler_;
}

bool RestServer::RegisterService(RestServicePtr service,
                                 const std::string& url) {
  return request_handler_->RegisterService(service, url);
}

}  // namespace webcc
