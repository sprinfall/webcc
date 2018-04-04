#ifndef WEBCC_SOAP_SERVER_H_
#define WEBCC_SOAP_SERVER_H_

// HTTP server handling SOAP requests.

#include <map>
#include <string>

#include "webcc/http_request_handler.h"
#include "webcc/http_server.h"

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

class SoapRequestHandler : public HttpRequestHandler {
public:
  SoapRequestHandler() = default;

  // Register a SOAP service to the given URL path.
  // \url URL path, must start with "/". E.g., "/calculator".
  // NOTE: Registering to the same URL multiple times is allowed, but only the
  //       last one takes effect.
  bool RegisterService(SoapServicePtr service, const std::string& url);

private:
  HttpStatus::Enum HandleSession(HttpSessionPtr session) override;

  SoapServicePtr GetServiceByUrl(const std::string& url);

private:
  typedef std::map<std::string, SoapServicePtr> UrlServiceMap;
  UrlServiceMap url_service_map_;
};

////////////////////////////////////////////////////////////////////////////////

class SoapServer : public HttpServer {
public:
  SoapServer(unsigned short port, std::size_t workers);

  ~SoapServer() override;

  bool RegisterService(SoapServicePtr service, const std::string& url);

private:
  SoapRequestHandler* soap_request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_SERVER_H_
