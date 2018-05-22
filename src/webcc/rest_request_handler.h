#ifndef WEBCC_REST_REQUEST_HANDLER_H_
#define WEBCC_REST_REQUEST_HANDLER_H_

// HTTP server handling REST requests.

#include "webcc/http_request_handler.h"
#include "webcc/rest_service_manager.h"

namespace webcc {

class RestRequestHandler : public HttpRequestHandler {
 public:
  ~RestRequestHandler() override = default;

  bool Bind(RestServicePtr service, const std::string& url, bool is_regex);

 private:
  void HandleSession(HttpSessionPtr session) override;

  RestServiceManager service_manager_;
};

}  // namespace webcc

#endif  // WEBCC_REST_REQUEST_HANDLER_H_
