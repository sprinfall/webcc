#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <string>

#include "webcc/globals.h"

namespace webcc {
namespace utility {

// Get a randomly generated UUID.
std::string RandomUuid();

// Get default user agent for HTTP headers.
const std::string& UserAgent();

// Get the timestamp for HTTP Date header field.
// E.g., Wed, 21 Oct 2015 07:28:00 GMT
// See: https://tools.ietf.org/html/rfc7231#section-7.1.1.2
std::string GetTimestamp();

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
bool SplitKV(const std::string& str, char delimiter,
             std::string* key, std::string* value);

// Convert string to size_t.
bool ToSize(const std::string& str, int base, std::size_t* size);

// Tell the size in bytes of the given file.
// Return kInvalidLength (-1) on failure.
std::size_t TellSize(const Path& path);

// Read entire file into string.
bool ReadFile(const Path& path, std::string* output);

// Dump the string data line by line to achieve more readability.
// Also limit the maximum size of the data to be dumped.
void DumpByLine(const std::string& data, std::ostream& os,
                const std::string& prefix);

}  // namespace utility
}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
