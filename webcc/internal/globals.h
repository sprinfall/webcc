#ifndef WEBCC_INTERNAL_GLOBALS_H_
#define WEBCC_INTERNAL_GLOBALS_H_

namespace webcc {
namespace internal {

const char* const kCRLF = "\r\n";

namespace literal_buffers {

// Buffers for composing payload.
// Literal strings can't be used because they have an extra '\0'.

const char HEADER_SEPARATOR[2] = { ':', ' ' };
const char CRLF[2] = { '\r', '\n' };
const char DOUBLE_DASHES[2] = { '-', '-' };

}  // namespace literal_buffers

namespace log_prefix {

const char* const kIncoming = "    < ";
const char* const kOutgoing = "    > ";

}  // namespace log_prefix

}  // namespace internal
}  // namespace webcc

#endif  // WEBCC_INTERNAL_GLOBALS_H_
