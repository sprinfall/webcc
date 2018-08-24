#include "webcc/rest_service.h"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

void RestListService::Handle(const RestRequest& request,
                             RestResponse* response) {
  if (request.method == kHttpGet) {
    Get(UrlQuery(request.url_query_str), response);
  } else if (request.method == kHttpPost) {
    Post(request.content, response);
  } else {
    LOG_ERRO("RestListService doesn't support '%s' method.",
             request.method.c_str());
  }
}

// -----------------------------------------------------------------------------

void RestDetailService::Handle(const RestRequest& request,
                               RestResponse* response) {
  if (request.method == kHttpGet) {
    Get(request.url_sub_matches, UrlQuery(request.url_query_str), response);
  } else if (request.method == kHttpPut) {
    Put(request.url_sub_matches, request.content, response);
  } else if (request.method == kHttpPatch) {
    Patch(request.url_sub_matches, request.content, response);
  } else if (request.method == kHttpDelete) {
    Delete(request.url_sub_matches, response);
  } else {
    LOG_ERRO("RestDetailService doesn't support '%s' method.",
             request.method.c_str());
  }
}

}  // namespace webcc
