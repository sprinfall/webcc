#include "webcc/router.h"

#include <algorithm>

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"

namespace webcc {

bool Router::Route(string_view url, ViewPtr view, const Strings& methods) {
  assert(view);

  // TODO: More error check

  routes_.push_back({ ToString(url), {}, view, methods });

  return true;
}

bool Router::Route(const UrlRegex& regex_url, ViewPtr view,
                   const Strings& methods) {
  assert(view);

  // TODO: More error check

  try {
    routes_.push_back({ "", regex_url(), view, methods });

  } catch (const std::regex_error& e) {
    LOG_ERRO("Not a valid regular expression: %s", e.what());
    return false;
  }

  return true;
}

ViewPtr Router::FindView(const std::string& method, const std::string& url,
                         UrlArgs* args) {
  assert(args != nullptr);

  for (auto& route : routes_) {
    if (std::find(route.methods.begin(), route.methods.end(), method) ==
        route.methods.end()) {
      continue;
    }

    if (route.url.empty()) {
      std::smatch match;

      if (std::regex_match(url, match, route.url_regex)) {
        // Any sub-matches?
        // Start from 1 because match[0] is the whole string itself.
        for (size_t i = 1; i < match.size(); ++i) {
          args->push_back(match[i].str());
        }

        return route.view;
      }
    } else {
      if (boost::iequals(route.url, url)) {
        return route.view;
      }
    }
  }

  return ViewPtr();
}

bool Router::MatchView(const std::string& method, const std::string& url,
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

      if (std::regex_match(url, match, route.url_regex)) {
        *stream = route.view->Stream(method);
        return true;
      }
    } else {
      if (boost::iequals(route.url, url)) {
        *stream = route.view->Stream(method);
        return true;
      }
    }
  }

  return false;
}

}  // namespace webcc
