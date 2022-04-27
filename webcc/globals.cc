#include "webcc/globals.h"

#include <iostream>

#include "boost/algorithm/string/case_conv.hpp"

namespace webcc {

// -----------------------------------------------------------------------------

namespace media_types {

std::string FromExtension(const std::string& ext) {
  std::string lext = boost::to_lower_copy(ext);

  // clang-format off
  if (lext == ".htm")   { return "text/html"; }
  if (lext == ".html")  { return "text/html"; }
  if (lext == ".php")   { return "text/html"; }
  if (lext == ".css")   { return "text/css"; }
  if (lext == ".txt")   { return "text/plain"; }
  if (lext == ".js")    { return "application/javascript"; }
  if (lext == ".json")  { return "application/json"; }
  if (lext == ".xml")   { return "application/xml"; }
  if (lext == ".swf")   { return "application/x-shockwave-flash"; }
  if (lext == ".flv")   { return "video/x-flv"; }
  if (lext == ".png")   { return "image/png"; }
  if (lext == ".jpe")   { return "image/jpeg"; }
  if (lext == ".jpeg")  { return "image/jpeg"; }
  if (lext == ".jpg")   { return "image/jpeg"; }
  if (lext == ".gif")   { return "image/gif"; }
  if (lext == ".bmp")   { return "image/bmp"; }
  if (lext == ".ico")   { return "image/vnd.microsoft.icon"; }
  if (lext == ".tiff")  { return "image/tiff"; }
  if (lext == ".tif")   { return "image/tiff"; }
  if (lext == ".svg")   { return "image/svg+xml"; }
  if (lext == ".svgz")  { return "image/svg+xml"; }
  // clang-format on

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
