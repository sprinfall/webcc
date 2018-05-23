#ifndef WEBCC_SOAP_SERVER_H_
#define WEBCC_SOAP_SERVER_H_

// HTTP server handling SOAP requests.

#include <string>

#include "webcc/soap_request_handler.h"
#include "webcc/http_server.h"

namespace webcc {

class SoapServer : public HttpServer {
 public:
  SoapServer(unsigned short port, std::size_t workers)
      : HttpServer(port, workers) {
  }

  ~SoapServer() override = default;

  // Bind a SOAP service to the given URL path.
  // The |url| path must start with "/", e.g., "/calculator".
  // Binding to the same URL multiple times is allowed, but only the last
  // one takes effect.
  bool Bind(SoapServicePtr service, const std::string& url) {
    return request_handler_.Bind(service, url);
  }

 private:
  HttpRequestHandler* GetRequestHandler() override {
    return &request_handler_;
  }

  SoapRequestHandler request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_SERVER_H_
