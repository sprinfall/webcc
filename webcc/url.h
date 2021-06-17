#ifndef WEBCC_URL_H_
#define WEBCC_URL_H_

#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

// A simple implementation of URL (or URI).
// TODO: Encoding of path
class Url {
public:
  // Encode URL different components.
  static std::string EncodeHost(string_view utf8_str);
  static std::string EncodePath(string_view utf8_str);
  static std::string EncodeQuery(string_view utf8_str);
  static std::string EncodeFull(string_view utf8_str);

public:
  Url() = default;

  explicit Url(string_view str, bool encode = false);

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

  void set_port(string_view port) {
    port_ = ToString(port);
  }

  // Append a piece of path.
  void AppendPath(string_view piece, bool encode = false);

  // Append a query parameter.
  void AppendQuery(string_view key, string_view value, bool encode = false);

private:
  void Parse(string_view str);

  void Clear();

private:
  std::string scheme_;
  std::string host_;
  std::string port_;
  std::string path_;
  std::string query_;
};

// -----------------------------------------------------------------------------

// For accessing URL query parameters.
class UrlQuery {
public:
  using Parameter = std::pair<std::string, std::string>;

  UrlQuery() = default;

  // The query string should be key-value pairs separated by '&'.
  explicit UrlQuery(const std::string& encoded_str);

  bool Empty() const {
    return parameters_.empty();
  }

  std::size_t Size() const {
    return parameters_.size();
  }

  bool Has(const std::string& key) const {
    return Find(key) != parameters_.end();
  }

  // Get a value by key.
  // Return empty string if the key doesn't exist.
  const std::string& Get(const std::string& key) const;

  // Get a key-value pair by index.
  const Parameter& Get(std::size_t index) const;

  void Add(const std::string& key, const std::string& value);

  void Remove(const std::string& key);

  // Return query string, encoded or not, joined with '&'.
  // E.g., "item=12731&color=blue&size=large".
  std::string ToString(bool encode = true) const;

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
  explicit UrlRegex(string_view url) : url_(url) {
  }

  std::regex operator()() const {
    std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;
    return std::regex{ url_, flags };
  }

private:
  std::string url_;
};

// Shortcut
using R = UrlRegex;

}  // namespace webcc

#endif  // WEBCC_URL_H_
