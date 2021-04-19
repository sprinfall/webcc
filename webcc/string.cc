#include "webcc/string.h"

#include <random>

#include "boost/algorithm/string/trim.hpp"

namespace webcc {

// Ref: https://stackoverflow.com/a/24586587
std::string RandomString(std::size_t length) {
  static const char chrs[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  thread_local static std::mt19937 rg{ std::random_device{}() };
  thread_local static std::uniform_int_distribution<std::string::size_type>
      pick{ 0, sizeof(chrs) - 2 };

  std::string s;
  s.reserve(length);

  while (length--) {
    s += chrs[pick(rg)];
  }

  return s;
}

bool ToSizeT(const std::string& str, int base, std::size_t* size) {
  try {
    *size = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

void Split(boost::string_view input, char delim, bool compress_token,
           std::vector<boost::string_view>* output) {
  std::size_t i = 0;
  std::size_t p = 0;

  i = input.find(delim);

  while (i != boost::string_view::npos) {
    output->emplace_back(input.substr(p, i - p));
    p = i + 1;

    if (compress_token) {
      while (input[p] == delim) {
        ++p;
      }
    }

    i = input.find(delim, p);
  }

  output->emplace_back(input.substr(p, i - p));
}

bool SplitKV(const std::string& input, char delim, bool trim_spaces,
             std::string* key, std::string* value) {
  std::size_t pos = input.find(delim);
  if (pos == std::string::npos) {
    return false;
  }

  *key = input.substr(0, pos);
  *value = input.substr(pos + 1);

  if (trim_spaces) {
    boost::trim(*key);
    boost::trim(*value);
  }

  return true;
}

}  // namespace webcc
