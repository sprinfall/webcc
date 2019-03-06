#include "webcc/soap_request_handler.h"

#include <utility>  // for move()

#include "webcc/logger.h"
#include "webcc/soap_globals.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

bool SoapRequestHandler::Bind(SoapServicePtr service, const std::string& url) {
  assert(service);
  url_service_map_[url] = service;
  return true;
}

void SoapRequestHandler::HandleConnection(HttpConnectionPtr connection) {
  SoapServicePtr service = GetServiceByUrl(connection->request().url().path());
  if (!service) {
    connection->SendResponse(http::Status::kBadRequest);
    return;
  }

  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(connection->request().content())) {
    connection->SendResponse(http::Status::kBadRequest);
    return;
  }

  SoapResponse soap_response;

  // Set SOAP envelope namespace according to SOAP version.
  // NOTE: This could be overwritten by service->Handle() anyway.
  if (soap_version_ == kSoapV11) {
    soap_response.set_soapenv_ns(kSoapEnvNamespaceV11);
  } else {
    soap_response.set_soapenv_ns(kSoapEnvNamespaceV12);
  }

  if (!service->Handle(soap_request, &soap_response)) {
    connection->SendResponse(http::Status::kBadRequest);
    return;
  }

  std::string content;
  soap_response.ToXml(format_raw_, indent_str_, &content);

  if (soap_version_ == kSoapV11) {
    connection->SetResponseContent(std::move(content),
                                http::media_types::kTextXml,
                                http::charsets::kUtf8);
  } else {
    connection->SetResponseContent(std::move(content),
                                http::media_types::kApplicationSoapXml,
                                http::charsets::kUtf8);
  }

  connection->SendResponse(http::Status::kOK);
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
