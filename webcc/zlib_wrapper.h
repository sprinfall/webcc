#ifndef WEBCC_ZLIB_WRAPPER_H_
#define WEBCC_ZLIB_WRAPPER_H_

#include <string>

namespace webcc {

bool Decompress(const std::string& input, std::string& output);

}  // namespace webcc

#endif  // WEBCC_ZLIB_WRAPPER_H_
