#include "webcc/http_response.h"

#include "webcc/utility.h"

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

bool HttpResponse::Prepare() {
  start_line_ = status_strings::ToString(status_);

  SetHeader("Server", http::UserAgent());
  SetHeader("Date", GetHttpDateTimestamp());

  // TODO: Support Keep-Alive.
  SetHeader(http::headers::kConnection, "Close");

  return true;
}

HttpResponse HttpResponse::Fault(http::Status status) {
  assert(status != http::Status::kOK);

  HttpResponse response;
  response.set_status(status);

  response.Prepare();

  return response;
}

}  // namespace webcc
