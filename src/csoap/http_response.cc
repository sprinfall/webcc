#include "csoap/http_response.h"

#include "csoap/common.h"
#include "csoap/xml.h"

namespace csoap {

std::ostream& operator<<(std::ostream& os, const HttpResponse& response) {
  os << response.start_line();

  for (const HttpHeader& h : response.headers_) {
    os << h.name << ": " << h.value << std::endl;
  }

  os << std::endl;

  // Pretty print the SOAP response XML.
  if (!xml::PrettyPrintXml(os, response.content())) {
    os << response.content();
  }

  return os;
}

namespace status_strings {

const std::string OK = "HTTP/1.1 200 OK\r\n";
const std::string BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const std::string INTERNAL_SERVER_ERROR = "HTTP/1.1 500 Internal Server Error\r\n";
const std::string NOT_IMPLEMENTED = "HTTP/1.1 501 Not Implemented\r\n";
const std::string SERVICE_UNAVAILABLE = "HTTP/1.1 503 Service Unavailable\r\n";

boost::asio::const_buffer ToBuffer(int status) {
  switch (status) {
    case HttpStatus::OK:
      return boost::asio::buffer(OK);

    case HttpStatus::BAD_REQUEST:
      return boost::asio::buffer(BAD_REQUEST);

    case HttpStatus::INTERNAL_SERVER_ERROR:
      return boost::asio::buffer(INTERNAL_SERVER_ERROR);

    case HttpStatus::NOT_IMPLEMENTED:
      return boost::asio::buffer(NOT_IMPLEMENTED);

    case HttpStatus::SERVICE_UNAVAILABLE:
      return boost::asio::buffer(SERVICE_UNAVAILABLE);

    default:
      return boost::asio::buffer(SERVICE_UNAVAILABLE);
  }
}

}  // namespace status_strings

namespace misc_strings {

const char NAME_VALUE_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

////////////////////////////////////////////////////////////////////////////////

// ATTENTION: The buffers don't hold the memory!
std::vector<boost::asio::const_buffer> HttpResponse::ToBuffers() const {
  std::vector<boost::asio::const_buffer> buffers;

  // Status line
  buffers.push_back(status_strings::ToBuffer(status_));

  // Header fields (optional)
  for (const HttpHeader& h : headers_) {
    buffers.push_back(boost::asio::buffer(h.name));
    buffers.push_back(boost::asio::buffer(misc_strings::NAME_VALUE_SEPARATOR));
    buffers.push_back(boost::asio::buffer(h.value));
    buffers.push_back(boost::asio::buffer(misc_strings::CRLF));
  }

  buffers.push_back(boost::asio::buffer(misc_strings::CRLF));

  // Content (optional)
  if (IsContentLengthValid()) {
    buffers.push_back(boost::asio::buffer(content_));
  }
  
  return buffers;
}

HttpResponse HttpResponse::Fault(HttpStatus status) {
  assert(status != HttpStatus::OK);

  HttpResponse response;
  response.set_status(status);

  return response;
}

}  // namespace csoap
