#include "webcc/string.h"

#include <random>

namespace webcc {
namespace string {

// See: https://stackoverflow.com/a/24586587
std::string RandomString(std::size_t length) {
  static const char chrs[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  thread_local static std::mt19937 rg{ std::random_device{}() };
  thread_local static std::uniform_int_distribution<std::string::size_type>
      pick(0, sizeof(chrs) - 2);

  std::string s;
  s.reserve(length);

  while (length--) {
    s += chrs[pick(rg)];
  }

  return s;
}

bool EqualsNoCase(const std::string& str1, const std::string& str2) {
  if (str1.size() != str2.size()) {
    return false;
  }

  return std::equal(str1.begin(), str1.end(), str2.begin(), [](int c1, int c2) {
    return std::toupper(c1) == std::toupper(c2);
  });
}

}  // namespace string
}  // namespace webcc
