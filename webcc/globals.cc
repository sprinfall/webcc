#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

// NOTE: Field names are case-insensitive.
//   See https://stackoverflow.com/a/5259004 for more details.
const std::string kHost = "Host";
const std::string kContentType = "Content-Type";
const std::string kContentLength = "Content-Length";

const std::string kAppJsonUtf8 = "application/json; charset=utf-8";

const std::string kHttpPort = "80";
const std::string kHttpSslPort = "443";

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
    case kXmlError:
      return "XML error";
    default:
      return "";
  }
}

}  // namespace webcc
