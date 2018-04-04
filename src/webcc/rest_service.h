#ifndef WEBCC_REST_SERVICE_H_
#define WEBCC_REST_SERVICE_H_

#include <memory>
#include <string>
#include <vector>

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
                      const std::vector<std::string>& url_sub_matches,
                      const std::string& request_content,
                      std::string* response_content) = 0;
};

typedef std::shared_ptr<RestService> RestServicePtr;

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
