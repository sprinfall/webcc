#ifndef WEBCC_REST_SERVICE_H_
#define WEBCC_REST_SERVICE_H_

#include <string>
#include <memory>

#include "webcc/common.h"

namespace webcc {

// Base class for your REST service.
class RestService {
public:
  virtual ~RestService() {
  }

  // Handle REST request, output the response.
  // Both the request and response parameters should be JSON.
  // TODO: Query parameters.
  virtual bool Handle(const std::string& http_method,
                      const std::string& request,
                      std::string* response) = 0;
};

typedef std::shared_ptr<RestService> RestServicePtr;

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
