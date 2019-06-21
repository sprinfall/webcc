#include "webcc/service.h"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

ResponsePtr ListService::Handle(RequestPtr request, const UrlArgs& args) {
  if (request->method() == methods::kGet) {
    return Get(UrlQuery(request->url().query()));
  }
  
  if (request->method() == methods::kPost) {
    return Post(request);
  }
  
  return ResponsePtr();
}

ResponsePtr ListService::Get(const UrlQuery& query) {
  return ResponsePtr();
}

ResponsePtr ListService::Post(RequestPtr request) {
  return ResponsePtr();
}

// -----------------------------------------------------------------------------

ResponsePtr DetailService::Handle(RequestPtr request, const UrlArgs& args) {
  if (request->method() == methods::kGet) {
    return Get(args, UrlQuery(request->url().query()));
  }
  
  if (request->method() == methods::kPut) {
    return Put(request, args);
  }
  
  if (request->method() == methods::kPatch) {
    return Patch(request, args);
  }

  if (request->method() == methods::kDelete) {
    return Delete(args);
  }

  return ResponsePtr();
}

ResponsePtr DetailService::Get(const UrlArgs& args, const UrlQuery& query) {
  return ResponsePtr();
}

ResponsePtr DetailService::Put(RequestPtr request, const UrlArgs& args) {
  return ResponsePtr();
}

ResponsePtr DetailService::Patch(RequestPtr request, const UrlArgs& args) {
  return ResponsePtr();
}

ResponsePtr DetailService::Delete(const UrlArgs& args) {
  return ResponsePtr();
}

}  // namespace webcc
