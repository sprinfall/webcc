#include "webcc/url.h"

#include <algorithm>
#include <sstream>

namespace webcc {

// -----------------------------------------------------------------------------
// Helper functions to decode URL string.

// Convert a hex character digit to a decimal character value.
static bool HexToDecimal(char hex, int* decimal) {
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

static bool Decode(const std::string& encoded, std::string* raw) {
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

// -----------------------------------------------------------------------------

UrlQuery::UrlQuery(const std::map<std::string, std::string>& map) {
  for (auto& pair : map) {
    Add(pair.first, pair.second);
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

  return str;
}

UrlQuery::ConstIterator UrlQuery::Find(const std::string& key) const {
  return std::find_if(parameters_.begin(),
                      parameters_.end(),
                      [&key](const SoapParameter& p) { return p.first == key; });
}

// -----------------------------------------------------------------------------

Url::Url(const std::string& str, bool decode) {
  if (!decode || str.find('%') == std::string::npos) {
    Init(str);
    return;
  }

  std::string decoded;
  if (Decode(str, &decoded)) {
    Init(decoded);
  } else {
    // TODO(Adam): Exception?
    Init(str);
  }
}

bool Url::IsValid() const {
  return !path_.empty();
}

std::vector<std::string> Url::SplitPath(const std::string& path) {
  std::vector<std::string> results;
  std::stringstream iss(path);
  std::string s;
  while (std::getline(iss, s, '/')) {
    if (!s.empty()) {
      results.push_back(s);
    }
  }
  return results;
}

static bool SplitKeyValue(const std::string& kv,
                          std::string* key, std::string* value) {
  std::size_t i = kv.find_first_of('=');
  if (i == std::string::npos || i == 0) {
    return false;
  }

  *key = kv.substr(0, i);
  *value = kv.substr(i + 1);
  return true;
}

// static
void Url::SplitQuery(const std::string& str, UrlQuery* query) {
  const std::size_t NPOS = std::string::npos;

  // Split into key value pairs separated by '&'.
  std::size_t i = 0;
  while (i != NPOS) {
    std::size_t j = str.find_first_of('&', i);

    std::string kv;
    if (j == NPOS) {
      kv = str.substr(i);
      i = NPOS;
    } else {
      kv = str.substr(i, j - i);
      i = j + 1;
    }

    std::string key;
    std::string value;
    if (SplitKeyValue(kv, &key, &value)) {
      query->Add(std::move(key), std::move(value));
    }
  }
}

void Url::Init(const std::string& str) {
  std::size_t pos = str.find('?');
  if (pos == std::string::npos) {
    path_ = str;
  } else {
    path_ = str.substr(0, pos);
    query_ = str.substr(pos + 1);
  }
}

}  // namespace webcc
