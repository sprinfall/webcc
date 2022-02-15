#ifndef WEBCC_STRING_H_
#define WEBCC_STRING_H_

#include <string>
#include <vector>

#include "webcc/globals.h"  // for string_view

namespace webcc {

#if (defined(_WIN32) || defined(_WIN64))
std::string Utf16To8(const std::wstring& utf16_string);
std::wstring Utf8To16(const std::string& utf8_string);
#endif

// Get a randomly generated string with the given length.
std::string RandomString(std::size_t length);

// Convert string to size_t.
// Just a wrapper of std::stoul.
bool ToSizeT(const std::string& str, int base, std::size_t* size);

void Trim(string_view& sv, const char* spaces = " ");

// Split string without copy.
// |compress_token| is the same as boost::token_compress_on for boost::split.
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

}  // namespace webcc

#endif  // WEBCC_STRING_H_
