#ifndef WEBCC_STRING_H_
#define WEBCC_STRING_H_

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

namespace webcc {

// Get a randomly generated string with the given length.
std::string random_string(std::size_t length);

// Convert string to size_t.
bool to_size_t(const std::string& str, int base, std::size_t* size);

inline std::string toupper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

inline std::string tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

inline bool iequals(const std::string& str1, const std::string& str2) {
  if (str1.size() != str2.size()) {
    return false;
  }

  return std::equal(str1.begin(), str1.end(), str2.begin(), [](int c1, int c2) {
    return std::toupper(c1) == std::toupper(c2);
  });
}

inline bool starts_with(const std::string& str, const std::string& sub) {
  if (sub.empty()) {
    return false;
  }
  return str.find(sub) == 0;
}

inline std::string& ltrim(std::string& str, const std::string& chars = "\t ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

inline std::string& rtrim(std::string& str, const std::string& chars = "\t ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

inline std::string& trim(std::string& str, const std::string& chars = "\t ") {
  return ltrim(rtrim(str, chars), chars);
}

// \param compress_token Same as boost::token_compress_on, especially useful
//                       when the delimeter is space.
template <class Container>
void split(Container& cont, const std::string& str, char delim = ' ',
           bool compress_token = false) {
  std::size_t i = 0;
  std::size_t p = 0;

  i = str.find(delim);

  while (i != std::string::npos) {
    cont.push_back(str.substr(p, i - p));
    p = i + 1;

    if (compress_token) {
      while (str[p] == delim) {
        ++p;
      }
    }

    i = str.find(delim, p);
  }

  cont.push_back(str.substr(p, i - p));
}

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
bool split_kv(std::string& key, std::string& value, const std::string& str,
              char delim, bool trim_spaces = true);

}  // namespace webcc

#endif  // WEBCC_STRING_H_
