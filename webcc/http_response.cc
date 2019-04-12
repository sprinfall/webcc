#include "webcc/http_response.h"

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
    case http::Status::kOK:
      return OK;

    case http::Status::kCreated:
      return CREATED;

    case http::Status::kAccepted:
      return ACCEPTED;

    case http::Status::kNoContent:
      return NO_CONTENT;

    case http::Status::kNotModified:
      return NOT_MODIFIED;

    case http::Status::kBadRequest:
      return BAD_REQUEST;

    case http::Status::kNotFound:
      return NOT_FOUND;

    case http::Status::kInternalServerError:
      return INTERNAL_SERVER_ERROR;

    case http::Status::kNotImplemented:
      return NOT_IMPLEMENTED;

    case http::Status::kServiceUnavailable:
      return SERVICE_UNAVAILABLE;

    default:
      return NOT_IMPLEMENTED;
  }
}

}  // namespace status_strings

void HttpResponse::Prepare() {
  if (start_line_.empty()) {
    start_line_ = status_strings::ToString(status_);
  }

  SetHeader(http::headers::kServer, http::UserAgent());
  SetHeader(http::headers::kDate, GetHttpDateTimestamp());

  HttpMessage::Prepare();
}

}  // namespace webcc
