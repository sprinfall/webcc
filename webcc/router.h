#ifndef WEBCC_ROUTER_H_
#define WEBCC_ROUTER_H_

#include <regex>
#include <string>

#include "webcc/globals.h"
#include "webcc/view.h"

namespace webcc {

struct RouteInfo {
  RouteInfo(std::string_view _url, ViewPtr _view,
            std::vector<std::string>&& _methods)
      : url(_url), view(_view), methods(std::move(_methods)) {
  }

  RouteInfo(std::regex&& _url_regex, ViewPtr _view,
            std::vector<std::string>&& _methods)
      : url_regex(std::move(_url_regex)),
        view(_view),
        methods(std::move(_methods)) {
  }

  std::string url;
  std::regex url_regex;
  ViewPtr view;
  std::vector<std::string> methods;
};

class Router {
public:
  virtual ~Router() = default;

  // Route a URL to a view.
  // The URL should start with "/". E.g., "/instances".
  bool Route(std::string_view url, ViewPtr view,
             std::vector<std::string>&& methods = { "GET" });

  // Route a URL (as regular expression) to a view.
  // The URL should start with "/" and be a regular expression.
  // E.g., "/instances/(\\d+)".
  bool Route(const UrlRegex& regex_url, ViewPtr view,
             std::vector<std::string>&& methods = { "GET" });

  // Find the view by HTTP method and URL path.
  ViewPtr FindView(const std::string& method, const std::string& url_path,
                   UrlArgs* args);

  // Match the view by HTTP method and URL path.
  // Return if a view is matched or not.
  // If the view asks for data streaming, `stream` will be set to true.
  bool MatchView(const std::string& method, const std::string& url_path,
                 bool* stream);

private:
  // Route table.
  std::vector<RouteInfo> routes_;
};

}  // namespace webcc

#endif  // WEBCC_ROUTER_H_
