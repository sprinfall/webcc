#ifndef CSOAP_HTTP_REQUEST_HANDLER_H_
#define CSOAP_HTTP_REQUEST_HANDLER_H_

#include <string>
#include <vector>

#include "csoap/soap_service.h"

namespace csoap {

class HttpRequest;
class HttpResponse;

// The common handler for all incoming requests.
class HttpRequestHandler {
public:
  HttpRequestHandler(const HttpRequestHandler&) = delete;
  HttpRequestHandler& operator=(const HttpRequestHandler&) = delete;

  HttpRequestHandler();

  bool RegisterService(SoapServicePtr soap_service);

  // Handle a request and produce a reply.
  void HandleRequest(const HttpRequest& request, HttpResponse& response);

private:
  std::vector<SoapServicePtr> soap_services_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_HANDLER_H_
