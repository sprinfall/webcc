#include "webcc/url.h"

#include <algorithm>
#include <cctype>
#include <functional>

#include "boost/algorithm/string/trim.hpp"

#include "webcc/string.h"
#include "webcc/utility.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace {

// Convert a hex character digit to a decimal character value.
bool HexToDecimal(char hex, int* decimal) {
  if (hex >= '0' && hex <= '9') {
    *decimal = hex - '0';
  } else if (hex >= 'A' && hex <= 'F') {
    *decimal = 10 + (hex - 'A');
  } else if (hex >= 'a' && hex <= 'f') {
    *decimal = 10 + (hex - 'a');
  } else {
    return false;
  }
  return true;
}

// Encode all characters which should be encoded.
std::string EncodeImpl(std::string_view raw,  // UTF8
                       std::function<bool(int)> should_encode) {
  const char kHex[] = "0123456789ABCDEF";

  std::string encoded;

  for (auto i = raw.begin(); i != raw.end(); ++i) {
    // For UTF8 encoded string, char ASCII can be greater than 127.
    // Cast to unsigned char firstly to make sure it's in [0,255].
    int c = static_cast<unsigned char>(*i);

    if (should_encode(c)) {
      encoded.push_back('%');
      encoded.push_back(kHex[(c >> 4) & 0xF]);
      encoded.push_back(kHex[c & 0xF]);
    } else {
      encoded.push_back(static_cast<char>(c));
    }
  }

  return encoded;
}

// Our own implementation of alpha numeric instead of std::isalnum to avoid
// taking locale into account.
inline bool IsAlNum(int c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

// Characters that are allowed in a URI but do not have a reserved purpose are
// are called unreserved. These include uppercase and lowercase letters, decimal
// digits, hyphen, period, underscore, and tilde.
inline bool IsUnreserved(int c) {
  return IsAlNum((unsigned char)c) || c == '-' || c == '.' || c == '_' ||
         c == '~';
}

// General delimiters serve as the delimiters between different uri components.
// General delimiters include:
// - All of these :/?#[]@
inline bool IsGenDelim(int c) {
  return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' ||
         c == '@';
}

// Sub-delimiters are those characters that may have a defined meaning within
// component of a URL/URI for a particular scheme. They do not serve as
// delimiters in any case between URL/URI segments. Sub-delimiters include:
// - All of these !$&'()*+,;=
bool IsSubDelim(int c) {
  switch (c) {
    case '!':
    case '$':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case ';':
    case '=':
      return true;
    default:
      return false;
  }
}

// Reserved characters includes the general delimiters and sub delimiters. Some
// characters are neither reserved nor unreserved, and must be percent-encoded.
inline bool IsReserved(int c) {
  return IsGenDelim(c) || IsSubDelim(c);
}

// Legal characters in the path portion include:
// - Any unreserved character
// - The percent character ('%'), and thus any percent-encoded octet
// - The sub-delimiters
// - ':' (colon)
// - '@' (at sign)
inline bool IsPathChar(int c) {
  return IsUnreserved(c) || IsSubDelim(c) ||
         c == '%' || c == '/' || c == ':' || c == '@';
}

// Legal characters in the query portion include:
// - Any path character
// - '?' (question mark)
inline bool IsQueryChar(int c) {
  return IsPathChar(c) || c == '?';
}

}  // namespace

// -----------------------------------------------------------------------------

std::string Url::EncodeHost(std::string_view utf8_str) {
  return EncodeImpl(utf8_str, [](int c) -> bool { return c > 127; });
}

std::string Url::EncodePath(std::string_view utf8_str) {
  return EncodeImpl(utf8_str, [](int c) -> bool {
    return !IsPathChar(c) || c == '%' || c == '+';
  });
}

std::string Url::EncodeQuery(std::string_view utf8_str) {
  return EncodeImpl(utf8_str, [](int c) -> bool {
    return !IsQueryChar(c) || c == '%' || c == '+';
  });
}

std::string Url::EncodeFull(std::string_view utf8_str) {
  return EncodeImpl(utf8_str, [](int c) -> bool {
    return !IsUnreserved(c) && !IsReserved(c);
  });
}

bool Url::Decode(std::string_view encoded, std::string* raw) {
  for (auto iter = encoded.begin(); iter != encoded.end(); ++iter) {
    if (*iter == '%') {
      if (++iter == encoded.end()) {
        // Invalid URI string, two hexadecimal digits must follow '%'.
        return false;
      }

      int h_decimal = 0;
      if (!HexToDecimal(*iter, &h_decimal)) {
        return false;
      }

      if (++iter == encoded.end()) {
        // Invalid URI string, two hexadecimal digits must follow '%'.
        return false;
      }

      int l_decimal = 0;
      if (!HexToDecimal(*iter, &l_decimal)) {
        return false;
      }

      raw->push_back(static_cast<char>((h_decimal << 4) + l_decimal));

    } else if (*iter > 127 || *iter < 0) {
      // Invalid encoded URI string, must be entirely ASCII.
      return false;
    } else {
      raw->push_back(*iter);
    }
  }

  return true;
}

std::string Url::DecodeUnsafe(std::string_view encoded) {
  std::string raw;
  if (Decode(encoded, &raw)) {
    return raw;
  }
  return std::string{ encoded };
}

// -----------------------------------------------------------------------------

Url::Url(std::string_view str, bool encode) {
  if (encode) {
    Parse(Url::EncodeFull(str));
  } else {
    Parse(str);
  }
}

void Url::AppendPath(std::string_view piece, bool encode) {
  if (piece.empty() || piece == "/") {
    return;
  }

  if (path_.empty() || path_ == "/") {
    path_.clear();
    if (piece.front() != '/') {
      path_.push_back('/');
    }
  } else if (path_.back() == '/' && piece.front() == '/') {
    path_.pop_back();
  } else if (path_.back() != '/' && piece.front() != '/') {
    path_.push_back('/');
  }

  if (encode) {
    path_.append(Url::EncodePath(piece));
  } else {
    path_.append(piece);
  }
}

void Url::AppendQuery(const std::string& key, const std::string& value,
                      bool encode) {
  if (!query_.empty()) {
    query_ += "&";
  }
  if (encode) {
    query_ += Url::EncodeQuery(key) + "=" + Url::EncodeQuery(value);
  } else {
    query_ += key + "=" + value;
  }
}

void Url::Parse(std::string_view str) {
  constexpr auto npos = std::string_view::npos;

  Trim(str);

  std::size_t p = npos;

  p = str.find("://");
  if (p != std::string::npos) {
    scheme_ = str.substr(0, p);
    str = str.substr(p + 3);
  }

  p = str.find('/');
  if (p != std::string::npos) {
    host_ = str.substr(0, p);

    str = str.substr(p);

    p = str.find('?');
    if (p != std::string::npos) {
      path_ = str.substr(0, p);
      query_ = str.substr(p + 1);
    } else {
      path_ = str;
    }
  } else {
    path_ = "";

    p = str.find('?');
    if (p != std::string::npos) {
      host_ = str.substr(0, p);
      query_ = str.substr(p + 1);
    } else {
      host_ = str;
    }
  }

  if (!host_.empty()) {
    // Check if there's a port.
    p = host_.find_last_of(':');
    if (p != std::string::npos) {
      // For IPv6: [::1]:8080
      std::size_t bracket = host_.find_last_of(']');
      if (bracket == std::string::npos || p > bracket) {
        port_ = host_.substr(p + 1);
        host_ = host_.substr(0, p);
      }
    }
  }
}

void Url::Clear() {
  scheme_.clear();
  host_.clear();
  port_.clear();
  path_.clear();
  query_.clear();
}

// -----------------------------------------------------------------------------

UrlQuery::UrlQuery(const std::string& encoded_str) {
  if (!encoded_str.empty()) {
    // Split into key value pairs separated by '&'.
    for (std::size_t i = 0; i != std::string::npos;) {
      std::size_t j = encoded_str.find_first_of('&', i);

      std::string kv;
      if (j == std::string::npos) {
        kv = encoded_str.substr(i);
        i = std::string::npos;
      } else {
        kv = encoded_str.substr(i, j - i);
        i = j + 1;
      }

      std::string_view key;
      std::string_view value;
      if (SplitKV(kv, '=', false, &key, &value)) {
        parameters_.push_back(
            { Url::DecodeUnsafe(key), Url::DecodeUnsafe(value) });
      }
    }
  }
}

const std::string& UrlQuery::Get(const std::string& key) const {
  auto it = Find(key);
  if (it != parameters_.end()) {
    return it->second;
  }

  static const std::string kEmptyValue;
  return kEmptyValue;
}

const UrlQuery::Parameter& UrlQuery::Get(std::size_t index) const {
  assert(index < Size());
  return parameters_[index];
}

void UrlQuery::Add(const std::string& key, const std::string& value) {
  if (!Has(key)) {
    parameters_.push_back({ key, value });
  }
}

void UrlQuery::Remove(const std::string& key) {
  auto it = Find(key);
  if (it != parameters_.end()) {
    parameters_.erase(it);
  }
}

std::string UrlQuery::ToString(bool encode) const {
  if (parameters_.empty()) {
    return "";
  }

  std::string str;

  for (std::size_t i = 0; i < parameters_.size(); ++i) {
    if (i != 0) {
      str += "&";
    }
    str += parameters_[i].first + "=" + parameters_[i].second;
  }

  if (encode) {
    return Url::EncodeQuery(str);
  } else {
    return str;
  }
}

UrlQuery::ConstIterator UrlQuery::Find(const std::string& key) const {
  return std::find_if(parameters_.begin(), parameters_.end(),
                      [&key](const Parameter& p) { return p.first == key; });
}

}  // namespace webcc
