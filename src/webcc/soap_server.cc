#include "webcc/soap_server.h"

#if WEBCC_DEBUG_OUTPUT
#include <iostream>
#endif

#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

bool SoapRequestHandler::RegisterService(SoapServicePtr service,
                                         const std::string& url) {
  assert(service);

  url_service_map_[url] = service;
  return true;
}

HttpStatus::Enum SoapRequestHandler::HandleSession(HttpSessionPtr session) {
  SoapServicePtr service = GetServiceByUrl(session->request().url());
  if (!service) {
    session->SetResponseStatus(HttpStatus::kBadRequest);
    session->SendResponse();
    return HttpStatus::kBadRequest;
  }

  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(session->request().content())) {
    session->SetResponseStatus(HttpStatus::kBadRequest);
    session->SendResponse();
    return HttpStatus::kBadRequest;
  }

  // TODO: Error handling.
  SoapResponse soap_response;
  service->Handle(soap_request, &soap_response);

  std::string content;
  soap_response.ToXml(&content);

  session->SetResponseStatus(HttpStatus::kOK);
  session->SetResponseContent(kTextXmlUtf8,
                              content.length(),
                              std::move(content));
  session->SendResponse();

  return HttpStatus::kOK;
}

SoapServicePtr SoapRequestHandler::GetServiceByUrl(const std::string& url) {
  UrlServiceMap::const_iterator it = url_service_map_.find(url);

  if (it != url_service_map_.end()) {
#if WEBCC_DEBUG_OUTPUT
    std::cout << "Service matches the URL: " << url << std::endl;
#endif
    return it->second;
  }

#if WEBCC_DEBUG_OUTPUT
  std::cout << "No service matches the URL: " << url << std::endl;
#endif

  return SoapServicePtr();
}

////////////////////////////////////////////////////////////////////////////////

SoapServer::SoapServer(unsigned short port, std::size_t workers)
    : HttpServer(port, workers)
    , soap_request_handler_(new SoapRequestHandler()) {
  request_handler_ = soap_request_handler_;
}

SoapServer::~SoapServer() {
  request_handler_ = NULL;
  delete soap_request_handler_;
}

bool SoapServer::RegisterService(SoapServicePtr service,
                                 const std::string& url) {
  return soap_request_handler_->RegisterService(service, url);
}

}  // namespace webcc
