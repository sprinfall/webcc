#ifndef WEBCC_URL_H_
#define WEBCC_URL_H_

// A simplified implementation of URL (or URI).
// The URL should start with "/".
// The parameters (separated by ";") are not supported.
// Example:
//   /inventory-check.cgi?item=12731&color=blue&size=large

#include <map>
#include <string>
#include <vector>

#include "webcc/common.h"

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

// URL query parameters.
class UrlQuery {
public:
  typedef std::vector<Parameter> Parameters;

  void Add(std::string&& key, std::string&& value);

  void Remove(const std::string& key);

  const std::string& GetValue(const std::string& key) const;

  bool HasKey(const std::string& key) const {
    return Find(key) != parameters_.end();
  }

private:
  typedef Parameters::const_iterator ConstIterator;
  ConstIterator Find(const std::string& key) const;

private:
  Parameters parameters_;
};

////////////////////////////////////////////////////////////////////////////////

class Url {
public:
  typedef std::map<std::string, std::string> Query;

  Url(const std::string& str);

  bool IsValid() const;

  const std::string& path() const {
    return path_;
  }

  void set_path(const std::string& path) {
    path_ = path;
  }

  const std::string& query() const {
    return query_;
  }

  void set_query(const std::string& query) {
    query_ = query;
  }

  // Split a path into its hierarchical components.
  static std::vector<std::string> SplitPath(const std::string& path);

  // Split query string into key-value parameters.
  static void SplitQuery(const std::string& str, UrlQuery* query);

private:
  std::string path_;
  std::string query_;
};

}  // namespace webcc

#endif  // WEBCC_URL_H_
