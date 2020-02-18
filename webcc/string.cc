#include "webcc/string.h"

#include <random>

namespace webcc {

// Ref: https://stackoverflow.com/a/24586587
std::string random_string(std::size_t length) {
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

bool to_size_t(const std::string& str, int base, std::size_t* size) {
  try {
    *size = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

bool split_kv(std::string& key, std::string& value, const std::string& str,
              char delim, bool trim_spaces) {
  std::size_t pos = str.find(delim);
  if (pos == std::string::npos) {
    return false;
  }

  key = str.substr(0, pos);
  value = str.substr(pos + 1);

  if (trim_spaces) {
    trim(key);
    trim(value);
  }

  return true;
}

}  // namespace webcc
