#ifndef WEBCC_VIEW_H_
#define WEBCC_VIEW_H_

#include <memory>

#include "webcc/request.h"
#include "webcc/response.h"

namespace webcc {

class View {
public:
  virtual ~View() = default;

  virtual ResponsePtr Handle(RequestPtr request) = 0;
};

using ViewPtr = std::shared_ptr<View>;

}  // namespace webcc

#endif  // WEBCC_VIEW_H_
