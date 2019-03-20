#ifndef WEBCC_ZLIB_WRAPPER_H_
#define WEBCC_ZLIB_WRAPPER_H_

#include <string>

namespace webcc {

// Compress the input string to gzip format output.
bool Compress(const std::string& input, std::string* output);

// Decompress the input string with auto detecting both gzip and zlib (deflate)
// formats.
bool Decompress(const std::string& input, std::string* output);

}  // namespace webcc

#endif  // WEBCC_ZLIB_WRAPPER_H_
