#include "webcc/url.h"

#include <algorithm>
#include <functional>
#include <sstream>

#include "boost/algorithm/string.hpp"

namespace webcc {

// -----------------------------------------------------------------------------
// Helper functions to decode URL string.

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

bool Decode(const std::string& encoded, std::string* raw) {
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

// Encodes all characters not in given set determined by given function.
std::string EncodeImpl(const std::string& raw,
                       std::function<bool(int)> should_encode) {
  const char* const hex = "0123456789ABCDEF";
  std::string encoded;

  for (auto iter = raw.begin(); iter != raw.end(); ++iter) {
    // For UTF8 encoded string, char ASCII can be greater than 127.
    int ch = static_cast<unsigned char>(*iter);

    // |ch| should be the same under both UTF8 and UTF16.
    if (should_encode(ch)) {
      encoded.push_back('%');
      encoded.push_back(hex[(ch >> 4) & 0xF]);
      encoded.push_back(hex[ch & 0xF]);
    } else {
      // ASCII doesn't need to be encoded, it should be the same under both
      // UTF8 and UTF16.
      encoded.push_back(static_cast<char>(ch));
    }
  }

  return encoded;
}

// Our own implementation of alpha numeric instead of std::isalnum to avoid
// taking global lock for performance reasons.
inline bool IsAlphaNumeric(char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

// Unreserved characters are those that are allowed in a URL/URI but do not have
// a reserved purpose. They include:
//   - A-Z
//   - a-z
//   - 0-9
//   - '-' (hyphen)
//   - '.' (period)
//   - '_' (underscore)
//   - '~' (tilde)
inline bool IsUnreserved(int c) {
  return IsAlphaNumeric((char)c) ||
         c == '-' || c == '.' || c == '_' || c == '~';
}

// Sub-delimiters are those characters that may have a defined meaning within
// component of a URL/URI for a particular scheme. They do not serve as
// delimiters in any case between URL/URI segments. Sub-delimiters include:
// - All of these !$&'()*+,;=
inline bool SubDelimiter(int c) {
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

inline bool IsPathChar(int c) {
  return IsUnreserved(c) || SubDelimiter(c) ||
         c == '%' || c == '/' || c == ':' || c == '@';
}

// Legal characters in the query portion include:
// - Any path character
// - '?' (question mark)
inline bool IsQueryChar(int c) {
  return IsPathChar(c) || c == '?';
}

// Encode the URL query string.
inline std::string EncodeQuery(const std::string& query) {
  return EncodeImpl(query, [](int c) {
    return !IsQueryChar(c) || c == '%' || c == '+';
  });
}

bool SplitKeyValue(const std::string& kv, std::string* key,
                   std::string* value) {
  std::size_t i = kv.find_first_of('=');
  if (i == std::string::npos || i == 0) {
    return false;
  }

  *key = kv.substr(0, i);
  *value = kv.substr(i + 1);
  return true;
}

}  // namespace

// -----------------------------------------------------------------------------

Url::Url(const std::string& str, bool decode) {
  Init(str, decode);
}

void Url::Init(const std::string& str, bool decode, bool clear) {
  if (clear) {
    Clear();
  }

  if (!decode || str.find('%') == std::string::npos) {
    Parse(str);
    return;
  }

  std::string decoded;
  if (Decode(str, &decoded)) {
    Parse(decoded);
  } else {
    // TODO: Exception?
    Parse(str);
  }
}

void Url::AddQuery(const std::string& key, const std::string& value) {
  if (!query_.empty()) {
    query_ += "&";
  }
  query_ += key + "=" + value;
}

void Url::Parse(const std::string& str) {
  std::string tmp = boost::trim_left_copy(str);

  std::size_t p = std::string::npos;

  p = tmp.find("://");
  if (p != std::string::npos) {
    scheme_ = tmp.substr(0, p);
    tmp = tmp.substr(p + 3);
  }

  p = tmp.find('/');
  if (p != std::string::npos) {
    host_ = tmp.substr(0, p);

    tmp = tmp.substr(p);

    p = tmp.find('?');
    if (p != std::string::npos) {
      path_ = tmp.substr(0, p);
      query_ = tmp.substr(p + 1);
    } else {
      path_ = tmp;
    }
  } else {
    path_ = "";

    p = tmp.find('?');
    if (p != std::string::npos) {
      host_ = tmp.substr(0, p);
      query_ = tmp.substr(p + 1);
    } else {
      host_ = tmp;
    }
  }

  if (!host_.empty()) {
    p = host_.find(':');
    if (p != std::string::npos) {
      port_ = host_.substr(p + 1);
      host_ = host_.substr(0, p);
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

UrlQuery::UrlQuery(const std::string& str) {
  if (!str.empty()) {
    // Split into key value pairs separated by '&'.
    for (std::size_t i = 0; i != std::string::npos;) {
      std::size_t j = str.find_first_of('&', i);

      std::string kv;
      if (j == std::string::npos) {
        kv = str.substr(i);
        i = std::string::npos;
      } else {
        kv = str.substr(i, j - i);
        i = j + 1;
      }

      std::string key;
      std::string value;
      if (SplitKeyValue(kv, &key, &value)) {
        Add(std::move(key), std::move(value));
      }
    }
  }
}

void UrlQuery::Add(std::string&& key, std::string&& value) {
  if (!Has(key)) {
    parameters_.push_back({ std::move(key), std::move(value) });
  }
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

const std::string& UrlQuery::Get(const std::string& key) const {
  auto it = Find(key);
  if (it != parameters_.end()) {
    return it->second;
  }

  static const std::string kEmptyValue;
  return kEmptyValue;
}

std::string UrlQuery::ToString() const {
  if (parameters_.empty()) {
    return "";
  }

  std::string str = parameters_[0].first + "=" + parameters_[0].second;

  for (std::size_t i = 1; i < parameters_.size(); ++i) {
    str += "&";
    str += parameters_[i].first + "=" + parameters_[i].second;
  }

  str = EncodeQuery(str);
  return str;
}

UrlQuery::ConstIterator UrlQuery::Find(const std::string& key) const {
  return std::find_if(parameters_.begin(), parameters_.end(),
                      [&key](const Parameter& p) { return p.first == key; });
}

}  // namespace webcc
