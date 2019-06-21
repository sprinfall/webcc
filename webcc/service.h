#ifndef WEBCC_SERVICE_H_
#define WEBCC_SERVICE_H_

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
#include "webcc/response.h"
#include "webcc/response_builder.h"
#include "webcc/url.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Regex sub-matches of the URL (usually resource ID's).
// Could also be considered as arguments, so named as UrlArgs.
using UrlArgs = std::vector<std::string>;

// -----------------------------------------------------------------------------

// Base class for your service.
class Service {
public:
  virtual ~Service() = default;

  // Handle request, return response.
  virtual ResponsePtr Handle(RequestPtr request, const UrlArgs& args) = 0;
};

using ServicePtr = std::shared_ptr<Service>;

// -----------------------------------------------------------------------------

class ListService : public Service {
public:
  ResponsePtr Handle(RequestPtr request, const UrlArgs& args) override;

protected:
  virtual ResponsePtr Get(const UrlQuery& query);

  virtual ResponsePtr Post(RequestPtr request);
};

// -----------------------------------------------------------------------------

class DetailService : public Service {
public:
  ResponsePtr Handle(RequestPtr request, const UrlArgs& args) override;

protected:
  virtual ResponsePtr Get(const UrlArgs& args, const UrlQuery& query);

  virtual ResponsePtr Put(RequestPtr request, const UrlArgs& args);

  virtual ResponsePtr Patch(RequestPtr request, const UrlArgs& args);

  virtual ResponsePtr Delete(const UrlArgs& args);
};

}  // namespace webcc

#endif  // WEBCC_SERVICE_H_
