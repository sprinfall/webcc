#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <iosfwd>
#include <string>

namespace webcc {

// Get the timestamp for HTTP Date header field.
// E.g., Wed, 21 Oct 2015 07:28:00 GMT
// See: https://tools.ietf.org/html/rfc7231#section-7.1.1.2
std::string GetHttpDateTimestamp();

std::string RandomUuid();

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
