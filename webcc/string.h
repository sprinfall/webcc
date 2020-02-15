#ifndef WEBCC_STRING_H_
#define WEBCC_STRING_H_

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

namespace webcc {
namespace string {

// Get a randomly generated string with the given length.
std::string RandomString(std::size_t length);

// TODO: What about std::wstring?
bool EqualsNoCase(const std::string& str1, const std::string& str2);

template <class Container>
void Split(const std::string& str, Container& cont) {
  std::istringstream iss(str);
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(), std::back_inserter(cont));
}

}  // namespace string
}  // namespace webcc

#endif  // WEBCC_STRING_H_
