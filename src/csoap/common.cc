#include "csoap/common.h"

#include "boost/lexical_cast.hpp"

namespace csoap {

// NOTE:
// Field names are case-insensitive.
// See: https://stackoverflow.com/a/5259004
const std::string kContentTypeName = "Content-Type";
const std::string kContentLengthName = "Content-Length";

// According to www.w3.org when placing SOAP messages in HTTP bodies, the HTTP
// Content-type header must be chosen as "application/soap+xml" [RFC 3902].
// But in practice, many web servers cannot understand it.
// See: https://www.w3.org/TR/2007/REC-soap12-part0-20070427/#L26854
const std::string kTextXmlUtf8 = "text/xml; charset=utf-8";

////////////////////////////////////////////////////////////////////////////////

const char* GetErrorMessage(ErrorCode error_code) {
  switch (error_code) {
    case kHostResolveError:
      return "Cannot resolve the host.";

    case kEndpointConnectError:
      return "Cannot connect to remote endpoint.";

    case kSocketTimeoutError:
      return "Operation timeout.";

    case kSocketReadError:
      return "Socket read error.";

    case kSocketWriteError:
      return "Socket write error.";

    case kHttpStartLineError:
      return "[HTTP Response] Start line is invalid.";

    case kHttpStatusError:
      return "[HTTP Response] Status is not OK.";

    case kHttpContentLengthError:
      return "[HTTP Response] Content-Length is invalid or missing.";

    case kXmlError:
      return "XML error";

    default:
      return "No error";
  }
}

////////////////////////////////////////////////////////////////////////////////

const Namespace kSoapEnvNamespace{
  "soap",
  "http://schemas.xmlsoap.org/soap/envelope/"
};

////////////////////////////////////////////////////////////////////////////////

Parameter::Parameter(const std::string& key, const std::string& value)
    : key_(key), value_(value) {
}

Parameter::Parameter(const std::string& key, int value)
    : key_(key) {
  value_ = boost::lexical_cast<std::string>(value);
}

Parameter::Parameter(const std::string& key, double value)
    : key_(key) {
  char buf[32];
  sprintf(buf, "%f", value);
  value_ = buf;
}

Parameter::Parameter(const std::string& key, bool value)
    : key_(key) {
  value_ = value ? "true" : "false";
}

}  // namespace csoap
