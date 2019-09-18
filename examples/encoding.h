#ifndef ENCODING_H_
#define ENCODING_H_

#include <string>

// Convert UTF16 to UTF8.
std::string Utf16ToUtf8(const std::wstring& utf16_string);

// Convert UTF8 to UTF16.
std::wstring Utf8ToUtf16(const std::string& utf8_string);

#endif  // ENCODING_H_
