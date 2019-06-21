#include "webcc/globals.h"

#include <iostream>

#include "boost/algorithm/string.hpp"

namespace webcc {

// -----------------------------------------------------------------------------

namespace media_types {

std::string FromExtension(const std::string& ext) {
  using boost::iequals;

  if (iequals(ext, ".htm"))   { return "text/html"; }
  if (iequals(ext, ".html"))  { return "text/html"; }
  if (iequals(ext, ".php"))   { return "text/html"; }
  if (iequals(ext, ".css"))   { return "text/css"; }
  if (iequals(ext, ".txt"))   { return "text/plain"; }
  if (iequals(ext, ".js"))    { return "application/javascript"; }
  if (iequals(ext, ".json"))  { return "application/json"; }
  if (iequals(ext, ".xml"))   { return "application/xml"; }
  if (iequals(ext, ".swf"))   { return "application/x-shockwave-flash"; }
  if (iequals(ext, ".flv"))   { return "video/x-flv"; }
  if (iequals(ext, ".png"))   { return "image/png"; }
  if (iequals(ext, ".jpe"))   { return "image/jpeg"; }
  if (iequals(ext, ".jpeg"))  { return "image/jpeg"; }
  if (iequals(ext, ".jpg"))   { return "image/jpeg"; }
  if (iequals(ext, ".gif"))   { return "image/gif"; }
  if (iequals(ext, ".bmp"))   { return "image/bmp"; }
  if (iequals(ext, ".ico"))   { return "image/vnd.microsoft.icon"; }
  if (iequals(ext, ".tiff"))  { return "image/tiff"; }
  if (iequals(ext, ".tif"))   { return "image/tiff"; }
  if (iequals(ext, ".svg"))   { return "image/svg+xml"; }
  if (iequals(ext, ".svgz"))  { return "image/svg+xml"; }

  return "application/text";
}

}  // namespace media_types

// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const Error& error) {
  os << "ERROR(";
  os << std::to_string(static_cast<int>(error.code()));
  os << "): " << error.message();
  if (error.timeout()) {
    os << " (timeout)";
  }
  return os;
}

}  // namespace webcc
