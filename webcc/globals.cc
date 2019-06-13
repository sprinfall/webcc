#include "webcc/globals.h"

#include <iostream>
#include <map>

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
