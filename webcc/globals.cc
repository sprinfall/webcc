#include "webcc/globals.h"

#include <map>

#include "webcc/version.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace media_types {

// TODO: Add more.
static void InitMap(std::map<std::string, std::string>& map) {
  map["gif"] = "image/gif";
  map["htm"] = "text/html";
  map["html"] = "text/html";
  map["jpg"] = "image/jpeg";
  map["jpeg"] = "image/jpeg";
  map["png"] = "image/png";
  map["txt"] = "text/plain";
  map[""] = "";
}

// TODO: Ignore case on Windows.
std::string FromExtension(const std::string& extension,
                          bool default_to_plain_text) {
  static std::map<std::string, std::string> s_map;

  if (s_map.empty()) {
    InitMap(s_map);
  }

  auto it = s_map.find(extension);
  if (it != s_map.end()) {
    return it->second;
  }

  if (default_to_plain_text) {
    return "text/plain";
  } else {
    return "";
  }
}

}  // namespace media_types

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
    case kFileIOError:
      return "File IO error";
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
