#include "csoap/common.h"

#include "boost/lexical_cast.hpp"

namespace csoap {

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
