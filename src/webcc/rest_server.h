#ifndef WEBCC_REST_SERVER_H_
#define WEBCC_REST_SERVER_H_

// HTTP server handling REST requests.

#include <string>

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

  // Bind a REST service to the given URL path.
  // The URL should start with "/" and it will be treated as a regular
  // expression if |is_regex| is true.
  // Examples:
  //   - "/instances"
  //   - "/instances/(\\d+)"
  // Binding to the same URL multiple times is allowed, but only the last one
  // takes effect.
  bool Bind(RestServicePtr service, const std::string& url, bool is_regex) {
    return request_handler_.Bind(service, url, is_regex);
  }

 private:
  HttpRequestHandler* GetRequestHandler() override {
    return &request_handler_;
  }

  RestRequestHandler request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVER_H_
