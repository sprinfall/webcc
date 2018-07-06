#include "webcc/soap_request_handler.h"

#include <utility>  // for move()

#include "webcc/logger.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

bool SoapRequestHandler::Bind(SoapServicePtr service, const std::string& url) {
  assert(service);

  url_service_map_[url] = service;
  return true;
}

void SoapRequestHandler::HandleConnection(HttpConnectionPtr connection) {
  SoapServicePtr service = GetServiceByUrl(connection->request().url());
  if (!service) {
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(connection->request().content())) {
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  SoapResponse soap_response;
  if (!service->Handle(soap_request, &soap_response)) {
    connection->SendResponse(HttpStatus::kBadRequest);
    return;
  }

  std::string content;
  soap_response.ToXml(&content);
  connection->SetResponseContent(std::move(content), kTextXmlUtf8);
  connection->SendResponse(HttpStatus::kOK);
}

SoapServicePtr SoapRequestHandler::GetServiceByUrl(const std::string& url) {
  UrlServiceMap::const_iterator it = url_service_map_.find(url);

  if (it != url_service_map_.end()) {
    LOG_VERB("Service matches the URL: %s", url.c_str());
    return it->second;
  }

  LOG_WARN("No service matches the URL: %s", url.c_str());

  return SoapServicePtr();
}

}  // namespace webcc
