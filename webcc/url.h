#ifndef WEBCC_URL_H_
#define WEBCC_URL_H_

// A simplified implementation of URL (or URI).

#include <string>
#include <utility>
#include <vector>

#include "webcc/globals.h"

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

  explicit Url(const std::string& str, bool decode = true);

  Url(Url&&) = default;
  Url& operator=(Url&&) = default;

  void Init(const std::string& str, bool decode = true, bool clear = true);

  const std::string& scheme() const {
    return scheme_;
  }

  const std::string& host() const {
    return host_;
  }

  const std::string& port() const {
    return port_;
  }

  const std::string& path() const {
    return path_;
  }

  const std::string& query() const {
    return query_;
  }

  // Add a parameter to the query string.
  void AddParameter(const std::string& key, const std::string& value);

private:
  void Parse(const std::string& str);

  void Clear();

  // TODO: Support auth & fragment.
  std::string scheme_;
  std::string host_;
  std::string port_;
  std::string path_;
  std::string query_;
};

}  // namespace webcc

#endif  // WEBCC_URL_H_
