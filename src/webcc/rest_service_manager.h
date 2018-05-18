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
  // The |url| should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  bool AddService(RestServicePtr service, const std::string& url);

  // The |sub_matches| is only available when the |url| bound to the
  // service is a regular expression and has sub-expressions.
  // E.g., the URL bound to the service is "/instances/(\\d+)", now match
  // "/instances/12345" against it, you will get one sub-match of "12345".
  RestServicePtr GetService(const std::string& url,
                            std::vector<std::string>* sub_matches);

private:
  class ServiceItem {
  public:
    ServiceItem(RestServicePtr _service, const std::string& _url)
      : service(_service), url(_url) {
    }

    ServiceItem(const ServiceItem& rhs) = default;
    ServiceItem& operator=(const ServiceItem& rhs) = default;

    ServiceItem(ServiceItem&& rhs)
        : url(std::move(rhs.url)),
          url_regex(std::move(rhs.url_regex)),
          service(rhs.service) {  // No move
    }

    RestServicePtr service;

    // URL string, e.g., "/instances/(\\d+)".
    std::string url;

    // Compiled regex for URL string.
    std::regex url_regex;
  };

  std::vector<ServiceItem> service_items_;

  DISALLOW_COPY_AND_ASSIGN(RestServiceManager);
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_MANAGER_H_
