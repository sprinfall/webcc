#ifndef WEBCC_SOAP_REQUEST_HANDLER_H_
#define WEBCC_SOAP_REQUEST_HANDLER_H_

#include <map>
#include <string>

#include "webcc/http_request_handler.h"

namespace webcc {

class SoapRequestHandler : public HttpRequestHandler {
 public:
  SoapRequestHandler() = default;
  ~SoapRequestHandler() override = default;

  bool Bind(SoapServicePtr service, const std::string& url);

 private:
  void HandleConnection(HttpConnectionPtr connection) override;

  SoapServicePtr GetServiceByUrl(const std::string& url);

  typedef std::map<std::string, SoapServicePtr> UrlServiceMap;
  UrlServiceMap url_service_map_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_REQUEST_HANDLER_H_
