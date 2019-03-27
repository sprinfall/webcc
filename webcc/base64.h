#ifndef WEBCC_BASE64_H_
#define WEBCC_BASE64_H_

#include <cctype>
#include <string>

namespace webcc {

std::string Base64Encode(const std::uint8_t* data, std::size_t len);

std::string Base64Encode(const std::string& input);

std::string Base64Decode(const std::string& data);

}  // namespace webcc

#endif  // WEBCC_BASE64_H_
