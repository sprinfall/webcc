#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

const std::string kHttpHead = "HEAD";
const std::string kHttpGet = "GET";
const std::string kHttpPost = "POST";
const std::string kHttpPatch = "PATCH";
const std::string kHttpPut = "PUT";
const std::string kHttpDelete = "DELETE";

// -----------------------------------------------------------------------------

const char* DescribeError(Error error) {
  switch (error) {
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

}  // namespace webcc
