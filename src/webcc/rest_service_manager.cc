#include "webcc/rest_service_manager.h"

#include <cassert>

#include "webcc/logger.h"

namespace webcc {

bool RestServiceManager::AddService(RestServicePtr service,
                                    const std::string& url) {
  assert(service);

  ServiceItem item(service, url);

  std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

  try {
    // Compile the regex.
    item.url_regex.assign(url, flags);

    service_items_.push_back(item);

    return true;

  } catch (std::regex_error& e) {
    LOG_ERRO("URL is not a valid regular expression: %s", e.what());
  }

  return false;
}

RestServicePtr RestServiceManager::GetService(
    const std::string& url,
    std::vector<std::string>* sub_matches) {

  assert(sub_matches != NULL);

  for (ServiceItem& item : service_items_) {
    std::smatch match;

    if (std::regex_match(url, match, item.url_regex)) {
      // Any sub-matches?
      // NOTE: Start from 1 because match[0] is the whole string itself.
      for (size_t i = 1; i < match.size(); ++i) {
        sub_matches->push_back(match[i].str());
      }

      return item.service;
    }
  }

  return RestServicePtr();
}

}  // namespace webcc
