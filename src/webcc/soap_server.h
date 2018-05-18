#ifndef WEBCC_SOAP_SERVER_H_
#define WEBCC_SOAP_SERVER_H_

// HTTP server handling SOAP requests.

#include "webcc/soap_request_handler.h"
#include "webcc/http_server.h"

namespace webcc {

class SoapServer : public HttpServer {
public:
  SoapServer(unsigned short port, std::size_t workers)
      : HttpServer(port, workers) {
  }

  ~SoapServer() override = default;

  bool RegisterService(SoapServicePtr service, const std::string& url) {
    return request_handler_.RegisterService(service, url);
  }

private:
  HttpRequestHandler* GetRequestHandler() override {
    return &request_handler_;
  }

  SoapRequestHandler request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_SERVER_H_
