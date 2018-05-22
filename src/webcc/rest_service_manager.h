#ifndef WEBCC_REST_SERVICE_MANAGER_H_
#define WEBCC_REST_SERVICE_MANAGER_H_

#include <regex>
#include <vector>

#include "webcc/rest_service.h"

namespace webcc {

class RestServiceManager {
 public:
  RestServiceManager() = default;

  // Add a service and bind it with the given URL.
  // The |url| should start with "/" and will be treated as a regular expression
  // if |regex| is true.
  // Examples: "/instances", "/instances/(\\d+)".
  bool AddService(RestServicePtr service, const std::string& url,
                  bool is_regex);

  // The |sub_matches| is only available when the |url| bound to the
  // service is a regular expression and has sub-expressions.
  // E.g., the URL bound to the service is "/instances/(\\d+)", now match
  // "/instances/12345" against it, you will get one sub-match of "12345".
  RestServicePtr GetService(const std::string& url,
                            std::vector<std::string>* sub_matches);

 private:
  class ServiceItem {
   public:
    ServiceItem(RestServicePtr _service, const std::string& _url,
                bool _is_regex)
        : service(_service), url(_url), is_regex(_is_regex) {
    }

    ServiceItem(const ServiceItem&) = default;
    ServiceItem& operator=(const ServiceItem&) = default;

    ServiceItem(ServiceItem&& rhs)
        : service(rhs.service),
          url(std::move(rhs.url)),
          is_regex(rhs.is_regex),
          url_regex(std::move(rhs.url_regex)) {
    }

    RestServicePtr service;

    // URL string, e.g., "/instances/(\\d+)".
    std::string url;

    // If the URL is a regular expression or not.
    bool is_regex;

    // Compiled regex for URL string.
    std::regex url_regex;
  };

  std::vector<ServiceItem> service_items_;

  DISALLOW_COPY_AND_ASSIGN(RestServiceManager);
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_MANAGER_H_
