#ifndef WEBCC_BASE64_H_
#define WEBCC_BASE64_H_

#include <cstdint>
#include <string>
#include <string_view>

namespace webcc {
namespace base64 {

std::string Encode(const std::uint8_t* data, std::size_t length);

std::string Encode(const std::string_view& input);

std::string Decode(const std::string_view& input);

}  // namespace base64
}  // namespace webcc

#endif  // WEBCC_BASE64_H_
