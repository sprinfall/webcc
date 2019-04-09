#include "webcc/globals.h"

#include <fstream>
#include <map>

#include "boost/filesystem/path.hpp"

#include "webcc/version.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Read entire file into string.
static bool ReadFile(const std::string& path, std::string* output) {
  std::ifstream ifs{path, std::ios::binary | std::ios::ate};
  if (!ifs) {
    return false;
  }

  auto size = ifs.tellg();
  output->resize((std::size_t)size, '\0');
  ifs.seekg(0);
  ifs.read(&(*output)[0], size);  // TODO: Error handling

  return true;
}

// -----------------------------------------------------------------------------

namespace http {

const std::string& UserAgent() {
  static std::string s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

File::File(const std::string& file_path) {
  if (!ReadFile(file_path, &data)) {
    throw Exception(kFileIOError, "Cannot read the file.");
  }

  namespace bfs = boost::filesystem;

  // Determine file name from file path.
  //if (file_name.empty()) {
    file_name = bfs::path(file_path).filename().string();
  //} else {
  //  file_name = file_name;
  //}

  // Determine content type from file extension.
  //if (mime_type.empty()) {
    std::string extension = bfs::path(file_path).extension().string();
    mime_type = http::media_types::FromExtension(extension, false);
  //} else {
    //mime_type = mime_type;
  //}
}

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
