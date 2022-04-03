#include "webcc/router.h"

#include <algorithm>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"

namespace webcc {

bool Router::Route(std::string_view url, ViewPtr view,
                   std::vector<std::string>&& methods) {
  assert(view);

  routes_.emplace_back(url, view, std::move(methods));

  return true;
}

bool Router::Route(const UrlRegex& regex_url, ViewPtr view,
                   std::vector<std::string>&& methods) {
  assert(view);

  try {
    routes_.emplace_back(regex_url(), view, std::move(methods));

  } catch (const std::regex_error& e) {
    LOG_ERRO("Not a valid regular expression: %s", e.what());
    return false;
  }

  return true;
}

ViewPtr Router::FindView(const std::string& method, const std::string& url_path,
                         UrlArgs* args) {
  assert(args != nullptr);

  for (auto& route : routes_) {
    if (std::find(route.methods.begin(), route.methods.end(), method) ==
        route.methods.end()) {
      continue;
    }

    if (route.url.empty()) {
      std::smatch match;

      if (std::regex_match(url_path, match, route.url_regex)) {
        // Any sub-matches?
        // Start from 1 because match[0] is the whole string itself.
        for (size_t i = 1; i < match.size(); ++i) {
          args->push_back(match[i].str());
        }

        return route.view;
      }
    } else {
      if (boost::iequals(route.url, url_path)) {
        return route.view;
      }
    }
  }

  return ViewPtr();
}

bool Router::MatchView(const std::string& method, const std::string& url_path,
                       bool* stream) {
  assert(stream != nullptr);
  *stream = false;

  for (auto& route : routes_) {
    if (std::find(route.methods.begin(), route.methods.end(), method) ==
        route.methods.end()) {
      continue;
    }

    if (route.url.empty()) {
      std::smatch match;
      if (std::regex_match(url_path, match, route.url_regex)) {
        *stream = route.view->Stream(method);
        return true;
      }
    } else {
      if (boost::iequals(route.url, url_path)) {
        *stream = route.view->Stream(method);
        return true;
      }
    }
  }

  return false;
}

}  // namespace webcc
