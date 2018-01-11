#include "csoap/http_request.h"

#include "boost/algorithm/string.hpp"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const HttpRequest& request) {
  return os << request.GetHeaders() << request.content();
}

////////////////////////////////////////////////////////////////////////////////

std::string HttpRequest::GetHeaders() const {
  std::string headers;

  // Start line

  headers += "POST ";
  headers += url_;
  headers += " ";

  if (version_ == kHttpV10) {
    headers += "HTTP/1.0";
  } else {
    headers += "HTTP/1.1";
  }
  headers += kCRLF;

  // Header fields

  headers += kContentTypeName;
  headers += ": ";

  if (!content_type_.empty()) {
    headers += content_type_;
  } else {
    headers += kTextXmlUtf8;
  }

  headers += kCRLF;

  headers += kContentLengthName;
  headers += ": ";
  headers += std::to_string(content_length_);
  headers += kCRLF;

  headers += "SOAPAction: ";
  headers += soap_action_;
  headers += kCRLF;

  headers += "Host: ";
  headers += host_;
  if (!port_.empty()) {
    headers += ":";
    headers += port_;
  }
  headers += kCRLF;

  headers += kCRLF;  // End of Headers.

  return headers;
}

}  // namespace csoap
