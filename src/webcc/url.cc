#include "webcc/url.h"

#include <algorithm>
#include <sstream>

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

void UrlQuery::Add(std::string&& key, std::string&& value) {
  if (!HasKey(key)) {
    parameters_.push_back({ std::move(key), std::move(value) });
  }
}

void UrlQuery::Remove(const std::string& key) {
  auto it = Find(key);
  if (it != parameters_.end()) {
    parameters_.erase(it);
  }
}

const std::string& UrlQuery::GetValue(const std::string& key) const {
  static const std::string kEmptyValue;

  auto it = Find(key);
  if (it != parameters_.end()) {
    return it->value();
  }
  return kEmptyValue;
}

UrlQuery::ConstIterator UrlQuery::Find(const std::string& key) const {
  return std::find_if(parameters_.begin(),
                      parameters_.end(),
                      [&key](const Parameter& p) { return p.key() == key; });
}

////////////////////////////////////////////////////////////////////////////////

Url::Url(const std::string& str) {
  std::size_t pos = str.find('?');
  if (pos == std::string::npos) {
    path_ = str;
  } else {
    path_ = str.substr(0, pos);
    query_ = str.substr(pos + 1);
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
                          std::string* key,
                          std::string* value) {
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

}  // namespace webcc
