#ifndef WEBCC_REST_SERVER_H_
#define WEBCC_REST_SERVER_H_

// HTTP server handling REST requests.

#include "webcc/http_server.h"
#include "webcc/rest_request_handler.h"
#include "webcc/rest_service.h"

namespace webcc {

class RestServer : public HttpServer {
public:
  RestServer(unsigned short port, std::size_t workers)
      : HttpServer(port, workers) {
  }

  ~RestServer() override = default;

  // Register a REST service to the given URL path.
  // The URL should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  // NOTE: Registering to the same URL multiple times is allowed, but only the
  //       last one takes effect.
  bool RegisterService(RestServicePtr service, const std::string& url) {
    return request_handler_.RegisterService(service, url);
  }

private:
  HttpRequestHandler* GetRequestHandler() override {
    return &request_handler_;
  }

private:
  RestRequestHandler request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVER_H_
