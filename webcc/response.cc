#include "webcc/response.h"

#include "webcc/utility.h"

namespace webcc {

namespace status_strings {

const std::string OK = "HTTP/1.1 200 OK";
const std::string CREATED = "HTTP/1.1 201 Created";
const std::string ACCEPTED = "HTTP/1.1 202 Accepted";
const std::string NO_CONTENT = "HTTP/1.1 204 No Content";
const std::string NOT_MODIFIED = "HTTP/1.1 304 Not Modified";
const std::string BAD_REQUEST = "HTTP/1.1 400 Bad Request";
const std::string NOT_FOUND = "HTTP/1.1 404 Not Found";
const std::string INTERNAL_SERVER_ERROR =
    "HTTP/1.1 500 Internal Server Error";
const std::string NOT_IMPLEMENTED = "HTTP/1.1 501 Not Implemented";
const std::string SERVICE_UNAVAILABLE = "HTTP/1.1 503 Service Unavailable";

const std::string& ToString(int status) {
  switch (status) {
    case Status::kOK:
      return OK;

    case Status::kCreated:
      return CREATED;

    case Status::kAccepted:
      return ACCEPTED;

    case Status::kNoContent:
      return NO_CONTENT;

    case Status::kNotModified:
      return NOT_MODIFIED;

    case Status::kBadRequest:
      return BAD_REQUEST;

    case Status::kNotFound:
      return NOT_FOUND;

    case Status::kInternalServerError:
      return INTERNAL_SERVER_ERROR;

    case Status::kNotImplemented:
      return NOT_IMPLEMENTED;

    case Status::kServiceUnavailable:
      return SERVICE_UNAVAILABLE;

    default:
      return NOT_IMPLEMENTED;
  }
}

}  // namespace status_strings

void Response::Prepare() {
  if (start_line_.empty()) {
    start_line_ = status_strings::ToString(status_);
  }

  SetHeader(headers::kServer, UserAgent());
  SetHeader(headers::kDate, GetTimestamp());

  Message::Prepare();
}

}  // namespace webcc
