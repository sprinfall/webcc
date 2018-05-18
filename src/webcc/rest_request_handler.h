#ifndef WEBCC_REST_REQUEST_HANDLER_H_
#define WEBCC_REST_REQUEST_HANDLER_H_

// HTTP server handling REST requests.

#include "webcc/http_request_handler.h"
#include "webcc/rest_service_manager.h"

namespace webcc {

class RestRequestHandler : public HttpRequestHandler {
public:
  ~RestRequestHandler() override = default;

  // Register a REST service to the given URL path.
  // The URL should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  bool RegisterService(RestServicePtr service, const std::string& url);

private:
  void HandleSession(HttpSessionPtr session) override;

private:
  RestServiceManager service_manager_;
};

}  // namespace webcc

#endif  // WEBCC_REST_REQUEST_HANDLER_H_
