#ifndef WEBCC_GZIP_H_
#define WEBCC_GZIP_H_

#include <string>

namespace webcc {
namespace gzip {

// Compress the input string to gzip format output.
bool Compress(const std::string& input, std::string* output);

// Decompress the input string with auto detecting both gzip and zlib (deflate)
// formats.
bool Decompress(const std::string& input, std::string* output);

}  // namespace gzip
}  // namespace webcc

#endif  // WEBCC_GZIP_H_
