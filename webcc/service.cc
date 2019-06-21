#include "webcc/service.h"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

void ListService::Handle(const RestRequest& request, RestResponse* response) {
  const std::string& method = request.http->method();

  if (method == methods::kGet) {
    Get(UrlQuery(request.http->url().query()), response);

  } else if (method == methods::kPost) {
    Post(request.http->content(), response);

  } else {
    LOG_ERRO("ListService doesn't support '%s' method.", method.c_str());
  }
}

// -----------------------------------------------------------------------------

void DetailService::Handle(const RestRequest& request, RestResponse* response) {
  const std::string& method = request.http->method();

  if (method == methods::kGet) {
    Get(request.url_matches, UrlQuery(request.http->url().query()), response);

  } else if (method == methods::kPut) {
    Put(request.url_matches, request.http->content(), response);

  } else if (method == methods::kPatch) {
    Patch(request.url_matches, request.http->content(), response);

  } else if (method == methods::kDelete) {
    Delete(request.url_matches, response);

  } else {
    LOG_ERRO("DetailService doesn't support '%s' method.", method.c_str());
  }
}

}  // namespace webcc
