#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <string>

namespace webcc {

std::string RandomUuid();

// Split a string to two parts by the given token.
bool Split2(const std::string& str, char token, std::string* part1,
            std::string* part2);

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
