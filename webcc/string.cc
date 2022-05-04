#include "webcc/string.h"

#ifdef _WIN32
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

#ifdef _WIN32

namespace windows_only {

// Wrapper for Windows API MultiByteToWideChar.
bool MB2WC(std::string_view input, unsigned int code_page,
           std::wstring* output) {
  const int input_size = static_cast<int>(input.size());
  if (input_size == 0) {
    return false;
  }

  DWORD flags = MB_PRECOMPOSED | MB_ERR_INVALID_CHARS;

  int size = ::MultiByteToWideChar(code_page, flags, input.data(), input_size,
                                   NULL, 0);

  if (size == 0) {
    return false;
  }

  output->resize(size);

  if (::MultiByteToWideChar(code_page, flags, input.data(), input_size,
                            output->data(), size) == 0) {
    return false;
  }

  return true;
}

// Wrapper for Windows API WideCharToMultiByte.
bool WC2MB(std::wstring_view input, unsigned int code_page,
           std::string* output) {
  const int input_size = static_cast<int>(input.size());
  if (input_size == 0) {
    return false;
  }

  // There do have other code pages which require the flags to be 0, e.g.,
  // 50220, 50211, and so on. But they are not included in our charset
  // dictionary. So, only consider CP_UTF8 (65001) and 54936 (GB18030).
  DWORD flags = 0;
  if (code_page == CP_UTF8 || code_page == 54936) {
    // WC_ERR_INVALID_CHARS:
    // Only applies when CodePage is specified as CP_UTF8 or 54936. It cannot be
    // used with other code page values. 
    flags = WC_ERR_INVALID_CHARS;
  }

  int size = ::WideCharToMultiByte(code_page, flags, input.data(), input_size,
                                   NULL, 0, NULL, NULL);

  if (size == 0) {
    return false;
  }

  output->resize(size);

  if (::WideCharToMultiByte(code_page, flags, input.data(), input_size,
                            output->data(), size, NULL, NULL) == 0) {
    return false;
  }

  return true;
}

bool WstrToUtf8(std::wstring_view wstr, std::string* utf8) {
  return WC2MB(wstr, CP_UTF8, utf8);
}

bool WstrToAnsi(std::wstring_view wstr, std::string* ansi) {
  return WC2MB(wstr, CP_ACP, ansi);
}

bool Utf8ToWstr(std::string_view utf8, std::wstring* wstr) {
  return MB2WC(utf8, CP_UTF8, wstr);
}

bool IsAsciiStr(std::wstring_view wstr) {
  for (std::size_t i = 0; i < wstr.size(); ++i) {
    if (wstr[i] > 127) {
      return false;
    }
  }
  return true;
}

}  // namespace windows_only

#endif  // _WIN32

}  // namespace webcc
