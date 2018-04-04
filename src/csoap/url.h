#ifndef CSOAP_URL_H_
#define CSOAP_URL_H_

// A simplified implementation of URL (or URI).
// The URL should start with "/".
// The parameters (separated by ";") are not supported.
// Example:
//   /inventory-check.cgi?item=12731&color=blue&size=large

#include <map>
#include <string>
#include <vector>

namespace csoap {

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

  // Split query string into key-value map.
  static Query SplitQuery(const std::string& query);

private:
  std::string path_;
  std::string query_;
};

}  // namespace csoap

#endif  // CSOAP_URL_H_
