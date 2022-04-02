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

void Split(std::string_view input, char delim, bool compress_token,
           std::vector<std::string_view>* output) {
  std::size_t off = 0;
  std::size_t pos = 0;

  while (true) {
    if (off == input.size()) {
      output->emplace_back(std::string_view{});
      break;
    }

    pos = input.find(delim, off);
    if (pos == std::string_view::npos) {
      output->emplace_back(input.substr(off));
      break;
    }

    output->emplace_back(input.substr(off, pos - off));

    off = pos + 1;

    if (compress_token) {
      while (off < input.size() && input[off] == delim) {
        ++off;
      }
    }
  }
}

void Trim(std::string_view& sv, const char* spaces) {
  sv.remove_prefix(std::min(sv.find_first_not_of(spaces), sv.size()));

  std::size_t pos = sv.find_last_not_of(spaces);
  if (pos != std::string_view::npos) {
    sv.remove_suffix(sv.size() - pos - 1);
  }
}

bool SplitKV(std::string_view input, char delim, bool trim_spaces,
             std::string_view* key, std::string_view* value) {
  std::size_t pos = input.find(delim);
  if (pos == std::string_view::npos) {
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

bool SplitKV(std::string_view input, char delim, bool trim_spaces,
             std::string* key, std::string* value) {
  std::string_view key_sv;
  std::string_view value_sv;
  if (SplitKV(input, delim, trim_spaces, &key_sv, &value_sv)) {
    *key = key_sv;
    *value = value_sv;
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
