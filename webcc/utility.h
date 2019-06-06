#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <string>

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

}  // namespace utility
}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
