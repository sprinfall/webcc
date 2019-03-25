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
  HttpRequestPtr http_request = connection->request();
  assert(http_request);

  auto http_response = std::make_shared<HttpResponse>();

  // TODO: Support keep-alive.
  http_response->SetHeader(http::headers::kConnection, "Close");

  std::string path = "/" + http_request->url().path();
  SoapServicePtr service = GetServiceByUrl(path);
  if (!service) {
    http_response->set_status(http::Status::kBadRequest);
    connection->SendResponse(http_response);
    return;
  }

  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(http_request->content())) {
    http_response->set_status(http::Status::kBadRequest);
    connection->SendResponse(http_response);
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
    http_response->set_status(http::Status::kBadRequest);
    connection->SendResponse(http_response);
    return;
  }

  std::string content;
  soap_response.ToXml(format_raw_, indent_str_, &content);

  // TODO: Let the service provide charset.
  if (soap_version_ == kSoapV11) {
    http_response->SetContentType(http::media_types::kTextXml,
                                  http::charsets::kUtf8);
  } else {
    http_response->SetContentType(http::media_types::kApplicationSoapXml,
                                  http::charsets::kUtf8);
  }

  http_response->set_status(http::Status::kOK);

  connection->SendResponse(http_response);
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
