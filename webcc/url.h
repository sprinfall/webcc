#ifndef WEBCC_URL_H_
#define WEBCC_URL_H_

// A simplified implementation of URL (or URI).
// The URL should start with "/".
// The parameters (separated by ";") are not supported.
// Example:
//   /inventory-check.cgi?item=12731&color=blue&size=large

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace webcc {

// -----------------------------------------------------------------------------

// URL query parameters.
class UrlQuery {
 public:
  typedef std::pair<std::string, std::string> Parameter;
  typedef std::vector<Parameter> Parameters;

  UrlQuery() = default;

  // The query string should be key value pairs separated by '&'.
  explicit UrlQuery(const std::string& str);

  // Construct from key-value pairs.
  explicit UrlQuery(const std::map<std::string, std::string>& map);

  void Add(const std::string& key, const std::string& value);

  void Add(std::string&& key, std::string&& value);

  void Remove(const std::string& key);

  // Get a value by key.
  // Return empty string if the key doesn't exist.
  const std::string& Get(const std::string& key) const;

  bool Has(const std::string& key) const {
    return Find(key) != parameters_.end();
  }

  bool IsEmpty() const {
    return parameters_.empty();
  }

  // Return key-value pairs concatenated by '&'.
  // E.g., "item=12731&color=blue&size=large".
  std::string ToString() const;

 private:
  typedef Parameters::const_iterator ConstIterator;
  ConstIterator Find(const std::string& key) const;

  Parameters parameters_;
};

// -----------------------------------------------------------------------------

class Url {
 public:
  Url() = default;
  Url(const std::string& str, bool decode);

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

 private:
  void Init(const std::string& str);

  std::string path_;
  std::string query_;
};

}  // namespace webcc

#endif  // WEBCC_URL_H_
