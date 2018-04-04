#include "csoap/url.h"

#include <sstream>

namespace csoap {

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
Url::Query Url::SplitQuery(const std::string& query) {
  const std::size_t NPOS = std::string::npos;

  Query result;

  // Split into key value pairs separated by '&'.
  std::size_t i = 0;
  while (i != NPOS) {
    std::size_t j = query.find_first_of('&', i);

    std::string kv;
    if (j == NPOS) {
      kv = query.substr(i);
      i = NPOS;
    } else {
      kv = query.substr(i, j - i);
      i = j + 1;
    }

    std::string key;
    std::string value;
    if (SplitKeyValue(kv, &key, &value)) {
      result[key] = value;  // TODO: Move
    }
  }

  return result;
}

}  // namespace csoap
