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
#include "webcc/url.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Regex sub-matches of the URL.
typedef std::vector<std::string> UrlSubMatches;

struct RestRequest {
  // HTTP method (GET, POST, etc.).
  const std::string& method;

  // Request content (JSON string).
  const std::string& content;

  // Query string of the URL (only for GET).
  const std::string& url_query_str;

  // Regex sub-matches of the URL (usually resource ID's).
  UrlSubMatches url_sub_matches;
};

struct RestResponse {
  http::Status status;
  std::string content;
};

// -----------------------------------------------------------------------------

// Base class for your REST service.
class RestService {
 public:
  virtual ~RestService() = default;

  // Handle REST request, output response.
  virtual void Handle(const RestRequest& request, RestResponse* response) = 0;
};

typedef std::shared_ptr<RestService> RestServicePtr;

// -----------------------------------------------------------------------------

class RestListService : public RestService {
 public:
  void Handle(const RestRequest& request, RestResponse* response) final;

 protected:
  RestListService() = default;

  virtual void Get(const UrlQuery& query, RestResponse* response) {
  }

  virtual void Post(const std::string& request_content,
                    RestResponse* response) {
  }
};

// -----------------------------------------------------------------------------

class RestDetailService : public RestService {
 public:
  void Handle(const RestRequest& request, RestResponse* response) final;

 protected:
  virtual void Get(const UrlSubMatches& url_sub_matches,
                   const UrlQuery& query,
                   RestResponse* response) {
  }

  virtual void Put(const UrlSubMatches& url_sub_matches,
                   const std::string& request_content,
                   RestResponse* response) {
  }

  virtual void Patch(const UrlSubMatches& url_sub_matches,
                     const std::string& request_content,
                     RestResponse* response) {
  }

  virtual void Delete(const UrlSubMatches& url_sub_matches,
                      RestResponse* response) {
  }
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
