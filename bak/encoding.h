#ifndef WEBCC_ENCODING_H_
#define WEBCC_ENCODING_H_

#include <string>
#include <codecvt>

namespace webcc {

std::string Utf16ToUtf8(const std::wstring& utf16_string) {
 std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
 return converter.to_bytes(utf16_string);
}

std::wstring Utf8ToUtf16(const std::string& utf8_string) {
 std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
 return converter.from_bytes(utf8_string);
}

}  // namespace webcc

#endif  // WEBCC_ENCODING_H_
