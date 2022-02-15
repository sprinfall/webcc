#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <iosfwd>
#include <string>

#include "boost/asio/ip/tcp.hpp"

#include "webcc/fs.h"
#include "webcc/globals.h"

namespace webcc {
namespace utility {

// Get default user agent for HTTP headers.
const std::string& UserAgent();

// Get the timestamp for HTTP Date header field.
// E.g., Wed, 21 Oct 2015 07:28:00 GMT
// See: https://tools.ietf.org/html/rfc7231#section-7.1.1.2
std::string HttpDate();

// Tell the size in bytes of the given file.
// Return kInvalidLength (-1) on failure.
std::size_t TellSize(const fs::path& path);

// Read entire file into string.
bool ReadFile(const fs::path& path, std::string* output);

// Dump the string data line by line to achieve more readability.
// Also limit the maximum size of the data to be dumped.
void DumpByLine(const std::string& data, std::ostream& os, string_view prefix);

// Print TCP endpoint.
// Usage: PrintEndpoint(std::cout, endpoint)
void PrintEndpoint(std::ostream& ostream,
                   const boost::asio::ip::tcp::endpoint& endpoint);

// TCP endpoint to string.
std::string EndpointToString(const boost::asio::ip::tcp::endpoint& endpoint);

// Translate a /-separated URL path to the local (relative) path.
// Examples:
//   (Non-Windows)
//   "/path/to/file" -> "path/to/file"
//   "/path/./to/../file" -> "path/to/file" (. and .. are ignored)
//   "/path//to//file" -> "path/to/file"
//   (Windows)
//   "/path/to/file" -> "path\to\file"
//   "/path\\sub/to/file" -> "to\file" (path\\sub is ignored)
//   "/C:\\test/path" -> "path" (C:\\test is ignored)
// Reference: Python http/server.py translate_path()
fs::path TranslatePath(const std::string& utf8_url_path);

}  // namespace utility
}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
