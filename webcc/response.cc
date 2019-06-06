#include "webcc/response.h"

#include "webcc/utility.h"

namespace webcc {

void Response::Prepare() {
  PrepareStatusLine();

  SetHeader(headers::kServer, utility::UserAgent());
  SetHeader(headers::kDate, utility::GetTimestamp());

  Message::Prepare();
}

static const std::pair<int, const char*> kTable[] = {
  { Status::kOK, "OK" },
  { Status::kCreated, "Created" },
  { Status::kAccepted, "Accepted" },
  { Status::kNoContent, "No Content" },
  { Status::kNotModified, "Not Modified" },
  { Status::kBadRequest, "Bad Request" },
  { Status::kNotFound, "Not Found" },
  { Status::kInternalServerError, "Internal Server Error" },
  { Status::kNotImplemented, "Not Implemented" },
  { Status::kServiceUnavailable, "Service Unavailable" },
};

static const char* GetReason(int status) {
  for (auto& pair : kTable) {
    if (pair.first == status) {
      return pair.second;
    }
  }
  return "";
}

void Response::PrepareStatusLine() {
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
}

}  // namespace webcc
