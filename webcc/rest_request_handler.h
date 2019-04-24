#ifndef WEBCC_REST_REQUEST_HANDLER_H_
#define WEBCC_REST_REQUEST_HANDLER_H_

// HTTP server handling REST requests.

#include <string>

#include "webcc/request_handler.h"
#include "webcc/rest_service_manager.h"

namespace webcc {

class RestRequestHandler : public RequestHandler {
public:
  RestRequestHandler() = default;

  ~RestRequestHandler() override = default;

  bool Bind(RestServicePtr service, const std::string& url, bool is_regex);

private:
  void HandleConnection(ConnectionPtr connection) override;

  void SetContent(RequestPtr request, ResponsePtr response,
                  std::string&& content);

private:
  RestServiceManager service_manager_;
};

}  // namespace webcc

#endif  // WEBCC_REST_REQUEST_HANDLER_H_
