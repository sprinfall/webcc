#ifndef WEBCC_ROUTER_H_
#define WEBCC_ROUTER_H_

#include <regex>
#include <string>

#include "webcc/globals.h"
#include "webcc/view.h"

namespace webcc {

class Router {
public:
  virtual ~Router() = default;

  // Route a URL to a view.
  // The URL should start with "/". E.g., "/instances".
  bool Route(string_view url, ViewPtr view,
             const Strings& methods = { "GET" });

  // Route a URL (as regular expression) to a view.
  // The URL should start with "/" and be a regular expression.
  // E.g., "/instances/(\\d+)".
  bool Route(const UrlRegex& regex_url, ViewPtr view,
             const Strings& methods = { "GET" });

  // Find the view by HTTP method and URL (path).
  ViewPtr FindView(const std::string& method, const std::string& url,
                   UrlArgs* args);

  // Match the view by HTTP method and URL path.
  // Return if a view is matched or not.
  // If the view asks for data streaming, |stream| will be set to true.
  bool MatchView(const std::string& method, const std::string& url_path,
                 bool* stream);

private:
  struct RouteInfo {
    std::string url;
    std::regex url_regex;
    ViewPtr view;
    Strings methods;
  };

  // Route table.
  std::vector<RouteInfo> routes_;
};

}  // namespace webcc

#endif  // WEBCC_ROUTER_H_
