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

void Split(string_view input, char delim, bool compress_token,
           std::vector<string_view>* output) {
  std::size_t i = 0;
  std::size_t p = 0;

  i = input.find(delim);

  while (i != string_view::npos) {
    output->emplace_back(input.substr(p, i - p));
    p = i + 1;

    if (compress_token) {
      while (p < input.size() && input[p] == delim) {
        ++p;
      }
    }

    i = input.find(delim, p);
  }

  output->emplace_back(input.substr(p, i - p));
}

void Trim(string_view& sv, const char* spaces) {
  sv.remove_prefix(std::min(sv.find_first_not_of(spaces), sv.size()));

  std::size_t pos = sv.find_last_not_of(spaces);
  if (pos != sv.npos) {
    sv.remove_suffix(sv.size() - pos - 1);
  }
}

bool SplitKV(string_view input, char delim, bool trim_spaces, string_view* key,
             string_view* value) {
  std::size_t pos = input.find(delim);
  if (pos == input.npos) {
    return false;
  }

  *key = input.substr(0, pos);
  *value = input.substr(pos + 1);

  if (trim_spaces) {
    Trim(*key);
    Trim(*value);
  }

  return true;
}

bool SplitKV(string_view input, char delim, bool trim_spaces, std::string* key,
             std::string* value) {
  string_view key_view;
  string_view value_view;
  if (SplitKV(input, delim, trim_spaces, &key_view, &value_view)) {
    *key = ToString(key_view);
    *value = ToString(value_view);
    return true;
  }
  return false;
}

}  // namespace webcc
