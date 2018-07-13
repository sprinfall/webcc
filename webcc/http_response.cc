#include "webcc/http_response.h"

namespace webcc {

namespace status_strings {

const std::string OK = "HTTP/1.1 200 OK\r\n";
const std::string CREATED = "HTTP/1.1 201 Created\r\n";
const std::string ACCEPTED = "HTTP/1.1 202 Accepted\r\n";
const std::string NO_CONTENT = "HTTP/1.1 204 No Content\r\n";
const std::string NOT_MODIFIED = "HTTP/1.1 304 Not Modified\r\n";
const std::string BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const std::string NOT_FOUND = "HTTP/1.1 404 Not Found\r\n";
const std::string INTERNAL_SERVER_ERROR =
    "HTTP/1.1 500 Internal Server Error\r\n";
const std::string NOT_IMPLEMENTED = "HTTP/1.1 501 Not Implemented\r\n";
const std::string SERVICE_UNAVAILABLE = "HTTP/1.1 503 Service Unavailable\r\n";

const std::string& ToString(int status) {
  switch (status) {
    case HttpStatus::kOK:
      return OK;

    case HttpStatus::kCreated:
      return CREATED;

    case HttpStatus::kAccepted:
      return ACCEPTED;

    case HttpStatus::kNoContent:
      return NO_CONTENT;

    case HttpStatus::kNotModified:
      return NOT_MODIFIED;

    case HttpStatus::kBadRequest:
      return BAD_REQUEST;

    case HttpStatus::kNotFound:
      return NOT_FOUND;

    case HttpStatus::InternalServerError:
      return INTERNAL_SERVER_ERROR;

    case HttpStatus::kNotImplemented:
      return NOT_IMPLEMENTED;

    case HttpStatus::kServiceUnavailable:
      return SERVICE_UNAVAILABLE;

    default:
      return NOT_IMPLEMENTED;
  }
}

}  // namespace status_strings

void HttpResponse::UpdateStartLine() {
  start_line_ = status_strings::ToString(status_);
}

HttpResponse HttpResponse::Fault(HttpStatus::Enum status) {
  assert(status != HttpStatus::kOK);

  HttpResponse response;
  response.set_status(status);
  response.UpdateStartLine();

  return response;
}

}  // namespace webcc
