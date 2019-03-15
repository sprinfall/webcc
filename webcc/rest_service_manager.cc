#include "webcc/rest_service_manager.h"

#include <cassert>

#include "webcc/logger.h"

namespace webcc {

bool RestServiceManager::AddService(RestServicePtr service,
                                    const std::string& url,
                                    bool is_regex) {
  assert(service);

  ServiceItem item(service, url, is_regex);

  if (!is_regex) {
    service_items_.push_back(std::move(item));
    return true;
  }

  std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

  try {
    // Compile the regex.
    item.url_regex.assign(url, flags);
    service_items_.push_back(std::move(item));
    return true;
  } catch (std::regex_error& e) {
    LOG_ERRO("URL is not a valid regular expression: %s", e.what());
    return false;
  }
}

RestServicePtr RestServiceManager::GetService(const std::string& url,
                                              UrlMatches* matches) {
  assert(matches != nullptr);

  for (ServiceItem& item : service_items_) {
    if (item.is_regex) {
      std::smatch match;

      if (std::regex_match(url, match, item.url_regex)) {
        // Any sub-matches?
        // NOTE: Start from 1 because match[0] is the whole string itself.
        for (size_t i = 1; i < match.size(); ++i) {
          matches->push_back(match[i].str());
        }

        return item.service;
      }
    } else {
      if (item.url == url) {
        return item.service;
      }
    }
  }

  return RestServicePtr();
}

}  // namespace webcc
