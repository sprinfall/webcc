#ifndef WEBCC_REST_SERVICE_H_
#define WEBCC_REST_SERVICE_H_

// NOTE:
// The design of RestListService and RestDetailService is very similar to
// XxxListView and XxxDetailView in Django Rest Framework.
// Deriving from them instead of RestService can simplify your own REST services
// a lot. But if you find the filtered parameters cannot meet your needs, feel
// free to derive from RestService directly.

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "webcc/common.h"

namespace webcc {

class UrlQuery;

////////////////////////////////////////////////////////////////////////////////

// Base class for your REST service.
class RestService {
public:
  virtual ~RestService() {
  }

  // Handle REST request, output the response.
  // \param http_method GET, POST, etc.
  // \param url_sub_matches The regex sub-matches in the URL,
  //                        usually resource ID.
  // \param query Query parameters in the URL.
  // \param request_content Request JSON.
  // \param response_content Output response JSON.
  virtual bool Handle(const std::string& http_method,
                      const std::vector<std::string>& url_sub_matches,
                      const UrlQuery& query,
                      const std::string& request_content,
                      std::string* response_content) = 0;
};

typedef std::shared_ptr<RestService> RestServicePtr;

////////////////////////////////////////////////////////////////////////////////

class RestListService : public RestService {
 public:
  bool Handle(const std::string& http_method,
              const std::vector<std::string>& url_sub_matches,
              const UrlQuery& query,
              const std::string& request_content,
              std::string* response_content) final;

 protected:
  RestListService() = default;

  virtual bool Get(const UrlQuery& query,
                   std::string* response_content) {
    return false;
  }

  virtual bool Post(const std::string& request_content,
                    std::string* response_content) {
    return false;
  }
};

////////////////////////////////////////////////////////////////////////////////

class RestDetailService : public RestService {
 public:
  bool Handle(const std::string& http_method,
              const std::vector<std::string>& url_sub_matches,
              const UrlQuery& query,
              const std::string& request_content,
              std::string* response_content) final;

 protected:
  virtual bool Get(const std::vector<std::string>& url_sub_matches,
                   std::string* response_content) {
    return false;
  }

  virtual bool Put(const std::vector<std::string>& url_sub_matches,
                   const std::string& request_content,
                   std::string* response_content) {
    return false;
  }

  virtual bool Patch(const std::vector<std::string>& url_sub_matches,
                     const std::string& request_content,
                     std::string* response_content) {
    return false;
  }

  virtual bool Delete(const std::vector<std::string>& url_sub_matches) {
    return false;
  }
};

}  // namespace webcc

#endif  // WEBCC_REST_SERVICE_H_
