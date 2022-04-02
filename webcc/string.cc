#include "webcc/string.h"

#if (defined(_WIN32) || defined(_WIN64))
#define NOMINMAX
#include <Windows.h>
#endif

#include <random>

namespace webcc {

// Ref: https://stackoverflow.com/a/24586587
std::string RandomAsciiString(std::size_t length) {
  static const char chrs[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  thread_local static std::mt19937 rg{ std::random_device{}() };
  thread_local static std::uniform_int_distribution<std::string::size_type>
      pick{ 0, sizeof(chrs) - 2 };

  std::string s;
  s.reserve(length);

  while (length-- > 0) {
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
  if (pos != string_view::npos) {
    sv.remove_suffix(sv.size() - pos - 1);
  }
}

bool SplitKV(string_view input, char delim, bool trim_spaces, string_view* key,
             string_view* value) {
  std::size_t pos = input.find(delim);
  if (pos == string_view::npos) {
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

#if (defined(_WIN32) || defined(_WIN64))

// Wrapper for Windows API MultiByteToWideChar.
static std::wstring MB2WC(const std::string& input, unsigned int code_page) {
  if (input.empty()) {
    return L"";
  }

  int length = ::MultiByteToWideChar(code_page, 0, &input[0],
                                     static_cast<int>(input.size()), NULL, 0);

  std::wstring output(length, '\0');

  ::MultiByteToWideChar(code_page, 0, &input[0], static_cast<int>(input.size()),
                        &output[0], static_cast<int>(output.size()));

  return output;
}

// Wrapper for Windows API WideCharToMultiByte.
static std::string WC2MB(const std::wstring& input, unsigned int code_page) {
  if (input.empty()) {
    return "";
  }

  // There do have other code pages which require the flags to be 0, e.g.,
  // 50220, 50211, and so on. But they are not included in our charset
  // dictionary. So, only consider 65001 (UTF-8) and 54936 (GB18030).
  DWORD flags = 0;
  if (code_page != 65001 && code_page != 54936) {
    flags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR;
  }

  int length = ::WideCharToMultiByte(code_page, flags, &input[0],
                                     static_cast<int>(input.size()), NULL, 0,
                                     NULL, NULL);

  std::string output(length, '\0');

  ::WideCharToMultiByte(code_page, flags, &input[0],
                        static_cast<int>(input.size()), &output[0],
                        static_cast<int>(output.size()), NULL, NULL);

  return output;
}

std::string Utf16To8(const std::wstring& utf16_string) {
  return WC2MB(utf16_string, CP_UTF8);
}

std::wstring Utf8To16(const std::string& utf8_string) {
  return MB2WC(utf8_string, CP_UTF8);
}

#endif  // defined(_WIN32) || defined(_WIN64)

}  // namespace webcc
