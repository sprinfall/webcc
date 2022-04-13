#ifndef WEBCC_BASE64_H_
#define WEBCC_BASE64_H_

#include <cstdint>
#include <string>
#include <string_view>

namespace webcc {
namespace base64 {

std::string Encode(const void* input, std::size_t size);

inline std::string Encode(std::string_view input) {
  return Encode(input.data(), input.size());
}

std::string Decode(std::string_view input);

}  // namespace base64
}  // namespace webcc

#endif  // WEBCC_BASE64_H_
