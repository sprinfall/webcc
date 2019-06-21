#ifndef WEBCC_SERVICE_MANAGER_H_
#define WEBCC_SERVICE_MANAGER_H_

#include <regex>  // NOLINT
#include <string>
#include <utility>  // for move()
#include <vector>

#include "webcc/service.h"

namespace webcc {

class ServiceManager {
public:
  ServiceManager() = default;

  ServiceManager(const ServiceManager&) = delete;
  ServiceManager& operator=(const ServiceManager&) = delete;

  // Add a service and bind it with the given URL.
  // The |url| should start with "/" and will be treated as a regular expression
  // if |regex| is true.
  // Examples: "/instances", "/instances/(\\d+)".
  bool AddService(ServicePtr service, const std::string& url, bool is_regex);

  // The |matches| is only available when the |url| bound to the service is a
  // regular expression and has sub-expressions.
  // E.g., the URL bound to the service is "/instances/(\\d+)", now match
  // "/instances/12345" against it, you will get one match of "12345".
  ServicePtr GetService(const std::string& url, UrlMatches* matches);

private:
  class Item {
  public:
    Item(ServicePtr _service, const std::string& _url, bool _is_regex)
        : service(_service), url(_url), is_regex(_is_regex) {
    }

    Item(const Item&) = default;
    Item& operator=(const Item&) = default;

    Item(Item&& rhs)
        : service(rhs.service),
          url(std::move(rhs.url)),
          is_regex(rhs.is_regex),
          url_regex(std::move(rhs.url_regex)) {
    }

    ServicePtr service;

    // URL string, e.g., "/instances/(\\d+)".
    std::string url;

    // If the URL is a regular expression or not.
    bool is_regex;

    // Compiled regex for URL string.
    std::regex url_regex;
  };

  std::vector<Item> items_;
};

}  // namespace webcc

#endif  // WEBCC_SERVICE_MANAGER_H_
