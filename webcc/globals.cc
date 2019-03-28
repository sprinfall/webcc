#include "webcc/globals.h"

#include "webcc/version.h"

namespace webcc {

namespace http {

const std::string& UserAgent() {
  static std::string s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

}  // namespace http

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
    case kFileIOError:
      return "File IO error";
    case kXmlError:
      return "XML error";
    default:
      return "";
  }
}

Exception::Exception(Error error, const std::string& details, bool timeout)
    : error_(error), msg_(DescribeError(error)), timeout_(timeout) {
  if (!details.empty()) {
    msg_ += " (" + details + ")";
  }
  if (timeout) {
    msg_ += " (timeout)";
  }
}

}  // namespace webcc
