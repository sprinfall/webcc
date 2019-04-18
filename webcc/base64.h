#ifndef WEBCC_BASE64_H_
#define WEBCC_BASE64_H_

#include <cstdint>
#include <string>

namespace webcc {

std::string Base64Encode(const std::uint8_t* data, std::size_t length);

std::string Base64Encode(const std::string& input);

std::string Base64Decode(const std::string& input);

}  // namespace webcc

#endif  // WEBCC_BASE64_H_
