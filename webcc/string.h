#ifndef WEBCC_STRING_H_
#define WEBCC_STRING_H_

#include <string>
#include <vector>

#include "webcc/config.h"

#if WEBCC_USE_STD_STRING_VIEW
#include <string_view>
#else
#include "boost/utility/string_view.hpp"
#endif  // WEBCC_USE_STD_STRING_VIEW

namespace webcc {

#if WEBCC_USE_STD_STRING_VIEW
using string_view = std::string_view;
#else
using string_view = boost::string_view;
#endif

inline std::string ToString(string_view sv) {
#if WEBCC_USE_STD_STRING_VIEW
  return std::string{ sv };
#else
  return sv.to_string();
#endif
}

// Generates randomly an ASCII string in the given length.
std::string RandomAsciiString(std::size_t length);

// Convert string to size_t.
// Just a wrapper of std::stoul.
bool ToSizeT(const std::string& str, int base, std::size_t* size);

void Trim(string_view& sv, const char* spaces = " ");

// Split string without copy.
// `compress_token` is the same as boost::token_compress_on for boost::split.
void Split(string_view input, char delim, bool compress_token,
           std::vector<string_view>* output);

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
bool SplitKV(string_view input, char delim, bool trim_spaces, string_view* key,
             string_view* value);

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
bool SplitKV(string_view input, char delim, bool trim_spaces,
             std::string* key, std::string* value);

// TODO
#if (defined(_WIN32) || defined(_WIN64))
std::string Utf16To8(const std::wstring& utf16_string);
std::wstring Utf8To16(const std::string& utf8_string);
#endif

}  // namespace webcc

#endif  // WEBCC_STRING_H_
