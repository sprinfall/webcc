#ifndef WEBCC_REST_SERVICE_H_
#define WEBCC_REST_SERVICE_H_

// NOTE:
// The design of RestListService and RestDetailService is very similar to
// XxxListView and XxxDetailView in Python Django Rest Framework.
// Deriving from them instead of RestService can simplify your own REST services
// a lot. But if you find the filtered parameters cannot meet your needs, feel
// free to derive from RestService directly.

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/url.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Regex sub-matches of the URL.
using UrlMatches = std::vector<std::string>;

struct RestRequest {
  // Original HTTP request.
  RequestPtr http;

  // Regex sub-matches of the URL (usually resource ID's).
  UrlMatches url_matches;
};

struct RestResponse {
  Status status;

  std::string content;

  std::string media_type;
  std::string charset;
};

// -----------------------------------------------------------------------------

// Base class for your REST service.
class RestService {
public:
  virtual ~RestService() = default;

  // Handle REST request, output response.
  virtual void Handle(const RestRequest& request, RestResponse* response) = 0;
};

using RestServicePtr = std::shared_ptr<RestService>;

// -----------------------------------------------------------------------------

class RestListService : public RestService {
public:
  void Handle(const RestRequest& request, RestResponse* response) override;

protected:
  virtual void Get(const UrlQuery& query, RestResponse* response) {
  }

  virtual void Post(const std::string& request_content,
                    RestResponse* response) {
  }
};

// -----------------------------------------------------------------------------

class RestDetailService : public RestService {
public:
  void Handle(const RestRequest& request, RestResponse* response) override;

protected:
  virtual void Get(const UrlMatches& url_matches,
                   const UrlQuery& query,
                   RestResponse* response) {
  }

  virtual void Put(const UrlMatches& url_matches,
                   const std::string& request_content,
                   RestResponse* response) {
  }

  virtual void Patch(const UrlMatches& url_matches,
                     const std::string& request_content,
                     RestResponse* response) {
  }

  virtual void Delete(const UrlMatches& url_matches,
                      RestResponse* response) {
  }
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
