#ifndef CSOAP_REST_SERVICE_H_
#define CSOAP_REST_SERVICE_H_

#include <string>
#include <memory>

#include "csoap/common.h"

namespace csoap {

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

}  // namespace csoap

#endif  // CSOAP_REST_SERVICE_H_
