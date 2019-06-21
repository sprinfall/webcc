#include "webcc/service_manager.h"

#include <cassert>

#include "webcc/logger.h"

namespace webcc {

bool ServiceManager::Add(ServicePtr service, const std::string& url,
                         bool is_regex) {
  assert(service);

  Item item(service, url, is_regex);

  if (!is_regex) {
    items_.push_back(std::move(item));
    return true;
  }

  std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

  try {
    // Compile the regex.
    item.url_regex.assign(url, flags);
    items_.push_back(std::move(item));
    return true;
  } catch (const std::regex_error& e) {
    LOG_ERRO("URL is not a valid regular expression: %s", e.what());
    return false;
  }
}

ServicePtr ServiceManager::Get(const std::string& url, UrlArgs* args) {
  assert(args != nullptr);

  for (Item& item : items_) {
    if (item.is_regex) {
      std::smatch match;

      if (std::regex_match(url, match, item.url_regex)) {
        // Any sub-matches?
        // NOTE: Start from 1 because match[0] is the whole string itself.
        for (size_t i = 1; i < match.size(); ++i) {
          args->push_back(match[i].str());
        }

        return item.service;
      }
    } else {
      if (item.url == url) {
        return item.service;
      }
    }
  }

  return ServicePtr();
}

}  // namespace webcc
