#ifndef WEBCC_STRING_H_
#define WEBCC_STRING_H_

#include <string>
#include <vector>

#include "boost/utility/string_view.hpp"

namespace webcc {

// Get a randomly generated string with the given length.
std::string RandomString(std::size_t length);

// Convert string to size_t.
// Just a wrapper of std::stoul.
bool ToSizeT(const std::string& str, int base, std::size_t* size);

// Split string without copy.
// |compress_token| is the same as boost::token_compress_on for boost::split.
void Split(boost::string_view input, char delim, bool compress_token,
           std::vector<boost::string_view>* output);

// Split a key-value string.
// E.g., split "Connection: Keep-Alive".
// TODO: Use string_view (blocked by trim)
bool SplitKV(const std::string& input, char delim, bool trim_spaces,
             std::string* key, std::string* value);

}  // namespace webcc

#endif  // WEBCC_STRING_H_
