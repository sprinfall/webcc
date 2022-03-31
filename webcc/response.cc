#include "webcc/response.h"

#include "webcc/utility.h"

namespace webcc {

using namespace status_codes;

static const std::pair<int, const char*> kTable[] = {
  { kOK, "OK" },
  { kCreated, "Created" },
  { kAccepted, "Accepted" },
  { kNoContent, "No Content" },
  { kNotModified, "Not Modified" },
  { kBadRequest, "Bad Request" },
  { kNotFound, "Not Found" },
  { kInternalServerError, "Internal Server Error" },
  { kNotImplemented, "Not Implemented" },
  { kServiceUnavailable, "Service Unavailable" },
};

static const char* GetReason(int status) {
  for (auto& pair : kTable) {
    if (pair.first == status) {
      return pair.second;
    }
  }
  return "";
}

void Response::Prepare() {
  if (!start_line_.empty()) {
    return;
  }

  start_line_ = "HTTP/1.1 ";
  start_line_ += std::to_string(status_);
  start_line_ += " ";

  if (reason_.empty()) {
    start_line_ += GetReason(status_);
  } else {
    start_line_ += reason_;
  }

  SetHeader(headers::kServer, utility::UserAgent());
}

}  // namespace webcc
