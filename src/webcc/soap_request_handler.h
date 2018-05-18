#ifndef WEBCC_SOAP_REQUEST_HANDLER_H_
#define WEBCC_SOAP_REQUEST_HANDLER_H_

#include <map>

#include "webcc/http_request_handler.h"

namespace webcc {

class SoapRequestHandler : public HttpRequestHandler {
public:
  SoapRequestHandler() = default;
  ~SoapRequestHandler() override = default;

  // Register a SOAP service to the given URL path.
  // The |url| path must start with "/", e.g., "/calculator".
  // Registering to the same URL multiple times is allowed, but only the last
  // one takes effect.
  bool RegisterService(SoapServicePtr service, const std::string& url);

private:
  void HandleSession(HttpSessionPtr session) override;

  SoapServicePtr GetServiceByUrl(const std::string& url);

  typedef std::map<std::string, SoapServicePtr> UrlServiceMap;
  UrlServiceMap url_service_map_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_REQUEST_HANDLER_H_
