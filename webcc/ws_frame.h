#ifndef WEBCC_WS_FRAME_H_
#define WEBCC_WS_FRAME_H_

// WebSocket data frame.

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "boost/endian/conversion.hpp"

#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace ws {

namespace status_codes {

constexpr std::uint16_t kNormalClosure = 1000;

}  // namespace status_codes

namespace opcodes {
constexpr byte_t kContinuationFrame = 0x0;
constexpr byte_t kTextFrame = 0x1;
constexpr byte_t kBinaryFrame = 0x2;
constexpr byte_t kConnectionClose = 0x8;
constexpr byte_t kPing = 0x9;
constexpr byte_t kPong = 0xA;
}  // namespace opcodes

// The number of bits for different kinds of payload length.
enum class PayloadLenBits { k7, k16, k63 };

// Max length when the payload length is 63 bits.
constexpr std::uint64_t kMaxLength63 = 0x7FFFFFFFFFFFFFFF;  // (1 << 63) - 1

// Generate a new masking key.
std::uint32_t NewMaskingKey();

// Transform the payload with the masking key.
void MaskTransform(byte_t* payload, std::size_t size,
                   const byte_t* masking_key);

// Set the high bit of a byte on or off.
inline byte_t SetHighBit(byte_t byte, bool on) {
  if (on) {
    return byte | 0x80;
  } else {
    return byte & 0x7F;
  }
}

// C++ replacements for ntohl, htonl, htons and ntohs.

template <typename T>
inline T NetworkToHost(T value) {
  return boost::endian::big_to_native(value);
}

template <typename T>
inline T HostToNetwork(T value) {
  return boost::endian::native_to_big(value);
}

template <typename T>
T NetworkToHost(const byte_t* bytes) {
  T value = 0;
  std::memcpy(&value, bytes, sizeof T);
  return boost::endian::big_to_native(value);
}

template <typename T>
void HostToNetwork(T value, byte_t* bytes) {
  boost::endian::native_to_big_inplace(value);
  std::memcpy(bytes, &value, sizeof T);
}

}  // namespace ws

class WSFrame;
std::ostream& operator<<(std::ostream& os, const WSFrame& frame);

// -----------------------------------------------------------------------------

class WSFrame {
public:
  WSFrame() = default;

  WSFrame(const WSFrame&) = default;
  WSFrame& operator=(const WSFrame&) = default;
  WSFrame(WSFrame&&) = default;
  WSFrame& operator=(WSFrame&&) = default;

  ~WSFrame() = default;

  bool Parse(const byte_t* bytes, std::size_t size, bool auto_unmask = true);

  bool ParseHeader(const byte_t* bytes, std::size_t size);

  void Build(bool fin, byte_t opcode, const byte_t* data, std::size_t size,
             const std::uint32_t* masking_key = nullptr);

  void Build(bool fin, std::string_view text, std::uint32_t masking_key);
  void Build(bool fin, std::string_view text);

  void Build(byte_t opcode);
  void Build(byte_t opcode, std::uint32_t masking_key);

  bool empty() const {
    return header_.empty();
  }

  std::size_t size() const {
    return header_.size() + payload_.size();
  }

  byte_t fin() const {
    return header_[0] >> 7;
  }

  void set_fin(bool fin) {
    header_[0] = ws::SetHighBit(header_[0], fin);
  }

  byte_t rsv1() const {
    return (header_[0] >> 6) & 0x01;
  }
  byte_t rsv2() const {
    return (header_[0] >> 5) & 0x01;
  }
  byte_t rsv3() const {
    return (header_[0] >> 4) & 0x01;
  }

  byte_t opcode() const {
    return header_[0] & 0x0F;
  }

  void set_opcode(byte_t opcode) {
    header_[0] = (header_[0] & 0xF0) | (opcode & 0x0F);
  }

  byte_t mask() const {
    return header_[1] >> 7;
  }

  bool masked() const {
    return mask() == 1;
  }

  void set_masked(bool masked) {
    header_[1] = ws::SetHighBit(header_[1], masked);
  }

  const byte_t* masking_key() const {
    assert(masked());
    return &header_[masking_key_off()];
  }

  std::size_t masking_key_off() const {
    return header_.size() - 4;
  }

  ws::PayloadLenBits payload_len_bits() const {
    return payload_len_bits_;
  }
 
  const byte_t* header() const {
    return header_.data();
  }

  std::size_t header_len() const {
    return header_.size();
  }

  const byte_t* payload() const {
    return payload_.data();
  }

  std::size_t payload_len() const {
    return payload_len_;
  }

  bool payload_masked() const {
    return payload_masked_;
  }

  void MaskPayload();
  void UnmaskPayload();

  void AddMask(std::uint32_t masking_key);

  std::string GetPayloadText() const;

  // Append payload data.
  // Return the number of appended bytes.
  std::size_t AppendPayload(const byte_t* data, std::size_t size);

  bool IsPayloadFull() const {
    return payload_.size() >= payload_len_;
  }

  // Get the payload for the socket to write.
  Payload GetPayload() const {
    using boost::asio::buffer;
    return Payload{ buffer(header_), buffer(payload_) };
  }

  // Convert to string for logging purpose.
  std::string Stringify() const;

private:
  std::vector<byte_t> header_;

  ws::PayloadLenBits payload_len_bits_ = ws::PayloadLenBits::k7;

  // Payload length according to the header.
  std::size_t payload_len_ = 0;

  // Payload data.
  std::vector<byte_t> payload_;

  bool payload_masked_ = false;
};

using WSFramePtr = std::shared_ptr<WSFrame>;

}  // namespace webcc

#endif  // WEBCC_WS_FRAME_H_
