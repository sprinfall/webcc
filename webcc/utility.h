#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <string>

namespace webcc {

std::string RandomUuid();

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
bool SplitKV(const std::string& str, char delimiter,
             std::string* key, std::string* value);

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
