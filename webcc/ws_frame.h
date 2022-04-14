#ifndef WEBCC_WS_FRAME_H_
#define WEBCC_WS_FRAME_H_

// WebSocket data frame.

#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "webcc/globals.h"

namespace webcc {

// -----------------------------------------------------------------------------

namespace ws {

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

}  // namespace ws

// -----------------------------------------------------------------------------

class WSFrame {
public:
  WSFrame() = default;

  WSFrame(const byte_t* bytes, std::size_t size, bool auto_unmask = true) {
    Init(bytes, size, auto_unmask);
  }

  // TODO: move
  WSFrame(const WSFrame& rhs);

  // TODO: move
  WSFrame& operator=(const WSFrame& rhs);

  ~WSFrame() = default;

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
  void RemoveMask();

  std::string GetPayloadText() const;

  // TODO:
  void Build(bool fin, std::string_view text, std::uint32_t masking_key);
  void Build(bool fin, std::string_view text);
  void Build(byte_t opcode);
  void Build(byte_t opcode, std::uint32_t masking_key);

  // Try to parse frame header from the given bytes.
  // Return whether the header is completely parsed or not.
  bool ParseHeader(const byte_t* data, std::size_t size);

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

  // Dump to output stream for logging purpose.
  void Dump(std::ostream& os) const;

  // Dump to string for logging purpose.
  std::string Dump() const;

private:
  bool Init(const byte_t* bytes, std::size_t size, bool auto_unmask);

  void Build(bool fin, const byte_t* data, std::size_t size,
             const std::uint32_t* masking_key = nullptr);

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
