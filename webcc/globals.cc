#include "webcc/globals.h"

#include "boost/algorithm/string.hpp"

namespace webcc {

// -----------------------------------------------------------------------------

const char* DescribeError(Error error) {
  switch (error) {
    case kSchemaError:
      return "Schema error";
    case kHostResolveError:
      return "Host resolve error";
    case kEndpointConnectError:
      return "Endpoint connect error";
    case kHandshakeError:
      return "Handshake error";
    case kSocketReadError:
      return "Socket read error";
    case kSocketWriteError:
      return "Socket write error";
    case kHttpError:
      return "HTTP error";
    case kServerError:
      return "Server error";
    case kXmlError:
      return "XML error";
    default:
      return "";
  }
}

Exception::Exception(Error error, bool timeout, const std::string& details)
    : error_(error), timeout_(timeout), msg_(DescribeError(error)) {
  if (timeout) {
    msg_ += " (timeout)";
  }
  if (!details.empty()) {
    msg_ += " (" + details + ")";
  }
}

}  // namespace webcc
