#ifndef WEBCC_REST_SERVICE_H_
#define WEBCC_REST_SERVICE_H_

#include <map>
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
  // \param http_method GET, POST, etc.
  // \param url_sub_matches The regex sub-matches in the URL,
  //                        usually resource ID.
  // \param query Query parameters in the URL, key value pairs.
  // \param request_content Request JSON.
  // \param response_content Output response JSON.
  virtual bool Handle(const std::string& http_method,
                      const std::vector<std::string>& url_sub_matches,
                      const std::map<std::string, std::string>& query,
                      const std::string& request_content,
                      std::string* response_content) = 0;
};

typedef std::shared_ptr<RestService> RestServicePtr;

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
