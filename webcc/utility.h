#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <iosfwd>
#include <string>

#include "webcc/globals.h"

namespace webcc {
namespace utility {

// Get default user agent for HTTP headers.
const std::string& UserAgent();

// Get the timestamp for HTTP Date header field.
// E.g., Wed, 21 Oct 2015 07:28:00 GMT
// See: https://tools.ietf.org/html/rfc7231#section-7.1.1.2
std::string HttpDate();

// Format a given time as HTTP date.
std::string FormatHttpDate(const std::tm& gmt);

// Tell the size in bytes of the given file.
// Return kInvalidSize on failure.
std::size_t TellSize(const sfs::path& path);

// Read the binary data of the file into an std::string.
bool ReadFile(const sfs::path& path, std::string* bytes);

// Write the binary data to the file.
bool WriteFile(const sfs::path& path, const std::string& bytes);

// Dump the string data line by line to achieve more readability.
// Also limit the maximum size of the data to be dumped.
void DumpByLine(const std::string& data, std::ostream& os,
                std::string_view prefix);

}  // namespace utility
}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
