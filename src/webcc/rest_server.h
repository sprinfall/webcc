#ifndef WEBCC_REST_SERVER_H_
#define WEBCC_REST_SERVER_H_

// HTTP server handling REST requests.

#include <regex>
#include <string>
#include <vector>

#include "webcc/http_request_handler.h"
#include "webcc/http_server.h"
#include "webcc/rest_service.h"

namespace webcc {

class Url;

////////////////////////////////////////////////////////////////////////////////

class RestServiceManager {
public:
  RestServiceManager() = default;
  RestServiceManager(const RestServiceManager&) = delete;
  RestServiceManager& operator=(const RestServiceManager&) = delete;

  // Add a service and bind it with the given URL.
  // The URL should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  bool AddService(RestServicePtr service, const std::string& url);

  // Parameter 'sub_matches' is only available when the URL bound to the
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
        : url(std::move(rhs.url))
        , url_regex(std::move(rhs.url_regex))
        , service(rhs.service) {  // No move
    }

    RestServicePtr service;

    // URL string, e.g., "/instances/(\\d+)".
    std::string url;

    // Compiled regex for URL string.
    std::regex url_regex;
  };

  std::vector<ServiceItem> service_items_;
};

////////////////////////////////////////////////////////////////////////////////

class RestRequestHandler : public HttpRequestHandler {
public:
  RestRequestHandler() = default;

  // Register a REST service to the given URL path.
  // The URL should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  bool RegisterService(RestServicePtr service, const std::string& url);

private:
  HttpStatus::Enum HandleSession(HttpSessionPtr session) override;

private:
  RestServiceManager service_manager_;
};

////////////////////////////////////////////////////////////////////////////////

class RestServer : public HttpServer {
public:
  RestServer(unsigned short port, std::size_t workers);

  ~RestServer() override;

  // Register a REST service to the given URL path.
  // The URL should start with "/" and could be a regular expression or not.
  // E.g., "/instances". "/instances/(\\d+)"
  // NOTE: Registering to the same URL multiple times is allowed, but only the
  //       last one takes effect.
  bool RegisterService(RestServicePtr service, const std::string& url);

private:
  RestRequestHandler* rest_request_handler_;
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVER_H_
