#include "webcc/rest_service.h"
#include "webcc/logger.h"

namespace webcc {

bool RestListService::Handle(const std::string& http_method,
                             const std::vector<std::string>& url_sub_matches,
                             const UrlQuery& query,
                             const std::string& request_content,
                             std::string* response_content) {
  if (http_method == kHttpGet) {
    return Get(query, response_content);
  }

  if (http_method == kHttpPost) {
    return Post(request_content, response_content);
  }

  LOG_ERRO("RestListService doesn't support '%s' method.", http_method.c_str());

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool RestDetailService::Handle(const std::string& http_method,
                               const std::vector<std::string>& url_sub_matches,
                               const UrlQuery& query,
                               const std::string& request_content,
                               std::string* response_content) {
  if (http_method == kHttpGet) {
    return Get(url_sub_matches, response_content);
  }

  if (http_method == kHttpPut) {
    return Put(url_sub_matches, request_content, response_content);
  }

  if (http_method == kHttpPatch) {
    return Patch(url_sub_matches, request_content, response_content);
  }

  if (http_method == kHttpDelete) {
    return Delete(url_sub_matches);
  }

  LOG_ERRO("RestDetailService doesn't support '%s' method.",
           http_method.c_str());

  return false;
}

}  // namespace webcc
