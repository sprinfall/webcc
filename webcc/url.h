#ifndef WEBCC_URL_H_
#define WEBCC_URL_H_

#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

// A simplified implementation of URL (or URI).
class Url {
public:
  Url() = default;

  explicit Url(const std::string& str, bool decode = true);

#if WEBCC_DEFAULT_MOVE_COPY_ASSIGN

  Url(Url&&) = default;
  Url& operator=(Url&&) = default;

#else

  Url(Url&& rhs)
      : scheme_(std::move(rhs.scheme_)),
        host_(std::move(rhs.host_)),
        port_(std::move(rhs.port_)),
        path_(std::move(rhs.path_)),
        query_(std::move(rhs.query_)) {
  }

  Url& operator=(Url&& rhs) {
    if (&rhs != this) {
      scheme_ = std::move(rhs.scheme_);
      host_ = std::move(rhs.host_);
      port_ = std::move(rhs.port_);
      path_ = std::move(rhs.path_);
      query_ = std::move(rhs.query_);
    }
    return *this;
  }

#endif  // WEBCC_DEFAULT_MOVE_COPY_ASSIGN

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

  // Add a query parameter.
  void AddQuery(const std::string& key, const std::string& value);

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

// -----------------------------------------------------------------------------

// URL query parameters.
class UrlQuery {
public:
  using Parameter = std::pair<std::string, std::string>;

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
  using ConstIterator = std::vector<Parameter>::const_iterator;

  ConstIterator Find(const std::string& key) const;

private:
  std::vector<Parameter> parameters_;
};

// -----------------------------------------------------------------------------

// Wrapper for URL as regular expression.
// Used by Server::Route().
class UrlRegex {
public:
  explicit UrlRegex(const std::string& url) : url_(url) {
  }

  std::regex operator()() const {
    std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

    return std::regex(url_, flags);
  }

private:
  std::string url_;
};

// Shortcut
using R = UrlRegex;

}  // namespace webcc

#endif  // WEBCC_URL_H_
