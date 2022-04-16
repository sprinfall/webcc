#include "webcc/ws_frame.h"

#include <sstream>

namespace webcc {

// -----------------------------------------------------------------------------

namespace ws {

static const char* OpCodeName(byte_t opcode) {
  switch (opcode) {
    case opcodes::kContinuationFrame:
      return "continuation frame";
    case opcodes::kTextFrame:
      return "text frame";
    case opcodes::kBinaryFrame:
      return "binary frame";
    case opcodes::kConnectionClose:
      return "connection close";
    case opcodes::kPing:
      return "ping";
    case opcodes::kPong:
      return "pong";
    default:
      return "";
  }
}

std::uint32_t NewMaskingKey() {
  std::uint32_t masking_key;

  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  std::uint8_t tmp[4];
  tmp[0] = std::rand() % 255;
  tmp[1] = std::rand() % 255;
  tmp[2] = std::rand() % 255;
  tmp[3] = std::rand() % 255;

  std::memcpy(&masking_key, tmp, 4);

  return masking_key;
}

void MaskTransform(byte_t* payload, std::size_t size,
                   const byte_t* masking_key) {
  for (std::size_t i = 0; i < size; ++i) {
    payload[i] ^= masking_key[i % 4];
  }
}

}  // namespace ws

// -----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const WSFrame& frame) {
  os << "fin: " << static_cast<int>(frame.fin()) << std::endl;
  os << "rsv1~3: " << static_cast<int>(frame.rsv1()) << ","
     << static_cast<int>(frame.rsv2()) << "," << static_cast<int>(frame.rsv3())
     << std::endl;
  os << "opcode: " << static_cast<int>(frame.opcode()) << " ("
     << ws::OpCodeName(frame.opcode()) << ")" << std::endl;
  os << "mask: " << static_cast<int>(frame.mask()) << std::endl;

  os << "payload length: " << frame.payload_len() << std::endl;

  return os;
}

// -----------------------------------------------------------------------------

bool WSFrame::Parse(const byte_t* bytes, std::size_t size, bool auto_unmask) {
  if (!ParseHeader(bytes, size)) {
    return false;
  }
  
  if ((payload_len_ + header_.size()) != size) {
    return false;
  }

  bytes += header_.size();

#if 1
  payload_.resize(payload_len_);
  std::memcpy(&payload_[0], bytes, payload_len_);
#else
  payload_.clear();
  payload_.insert(payload_.end(), bytes, bytes + payload_len_);
#endif

  if (masked() && auto_unmask) {
    UnmaskPayload();
  }

  return true;
}

bool WSFrame::ParseHeader(const byte_t* bytes, std::size_t size) {
  if (size < 2) {
    return false;
  }

  std::size_t header_size = 2;

  std::uint8_t byte1 = bytes[1];
  std::uint8_t payload_len7 = byte1 & 0x7F;

  if (payload_len7 < 126) {
    payload_len_bits_ = ws::PayloadLenBits::k7;
    payload_len_ = payload_len7;

  } else if (payload_len7 == 126) {
    payload_len_bits_ = ws::PayloadLenBits::k16;
    header_size += 2;

    if (size < header_size) {
      return false;
    }

    payload_len_ = ws::NetworkToHost<std::uint16_t>(&bytes[2]);

  } else if (payload_len7 == 127) {
    payload_len_bits_ = ws::PayloadLenBits::k63;
    header_size += 8;

    if (size < header_size) {
      return false;
    }

    payload_len_ = ws::NetworkToHost<std::uint64_t>(&bytes[2]);

    if (payload_len_ > ws::kMaxLength63) {
      // TODO: Error handling
    }
  }

  const bool masked = (bytes[1] >> 7) == 1;

  if (masked) {
    payload_masked_ = true;
    header_size += 4;

    if (size < header_size) {
      return false;
    }
  }

#if 1
  header_.resize(header_size);
  std::memcpy(&header_[0], bytes, header_size);
#else
  header_.clear();
  header_.insert(header_.end(), bytes, bytes + header_size);
#endif
 
  return true;
}

void WSFrame::Build(bool fin, byte_t opcode, const byte_t* data,
                    std::size_t size, const std::uint32_t* masking_key) {
  assert(empty());

  payload_len_ = size;

  if (payload_len_ < 126) {
    payload_len_bits_ = ws::PayloadLenBits::k7;
  } else if (payload_len_ < 0xFFFF) {
    payload_len_bits_ = ws::PayloadLenBits::k16;
  } else {
    if (payload_len_ <= ws::kMaxLength63) {
      payload_len_bits_ = ws::PayloadLenBits::k63;
    } else {
      // TODO: Error handling
    }
  }

  std::size_t header_size = 2;

  if (payload_len_bits_ == ws::PayloadLenBits::k16) {
    header_size += 2;
  } else if (payload_len_bits_ == ws::PayloadLenBits::k63) {
    header_size += 8;
  }

  if (masking_key != nullptr) {
    header_size += 4;
  }

  header_.resize(header_size);

  // Clear the first 2 bytes
  header_[0] = 0;
  header_[1] = 0;

  std::size_t off = 2;  // start

  set_fin(fin);
  set_opcode(opcode);

  if (payload_len_bits_ == ws::PayloadLenBits::k7) {
    header_[1] = static_cast<byte_t>(payload_len_);

  } else if (payload_len_bits_ == ws::PayloadLenBits::k16) {
    header_[1] = byte_t(126);
    ws::HostToNetwork(static_cast<std::uint16_t>(payload_len_), &header_[2]);
    off += 2;

  } else if (payload_len_bits_ == ws::PayloadLenBits::k63) {
    header_[1] = byte_t(127);
    ws::HostToNetwork(static_cast<std::uint64_t>(payload_len_), &header_[2]);
    off += 8;
  }

  if (masking_key != nullptr) {
    // No need to consider byte order.
    std::memcpy(&header_[off], masking_key, 4);
    off += 4;
    set_masked(true);
  }

  payload_.resize(payload_len_);
  if (payload_len_ > 0) {
    std::memcpy(&payload_[0], data, payload_len_);
  }

  payload_masked_ = false;

  if (masking_key != nullptr) {
    MaskPayload();
  }
}

void WSFrame::Build(bool fin, std::string_view text,
                    std::uint32_t masking_key) {
  const byte_t* data = reinterpret_cast<const byte_t*>(text.data());
  Build(fin, ws::opcodes::kTextFrame, data, text.size(), &masking_key);
}

void WSFrame::Build(bool fin, std::string_view text) {
  const byte_t* data = reinterpret_cast<const byte_t*>(text.data());
  Build(fin, ws::opcodes::kTextFrame, data, text.size(), nullptr);
}

void WSFrame::Build(byte_t opcode) {
  Build(true, opcode, nullptr, 0, nullptr);
}

void WSFrame::Build(byte_t opcode, std::uint32_t masking_key) {
  Build(true, opcode, nullptr, 0, &masking_key);
}

void WSFrame::MaskPayload() {
  if (!payload_masked_) {
    if (!payload_.empty()) {
      ws::MaskTransform(&payload_[0], payload_.size(), masking_key());
    }
    payload_masked_ = true;
  }
}

void WSFrame::UnmaskPayload() {
  if (payload_masked_) {
    if (!payload_.empty()) {
      ws::MaskTransform(&payload_[0], payload_.size(), masking_key());
    }
    payload_masked_ = false;
  }
}

void WSFrame::AddMask(std::uint32_t masking_key) {
  if (masked()) {
    UnmaskPayload();
    std::memcpy(&header_[masking_key_off()], &masking_key, 4);
    MaskPayload();
  } else {
    header_.insert(header_.end(), 4, 0);
    set_masked(true);
    std::memcpy(&header_[masking_key_off()], &masking_key, 4);
    MaskPayload();
  }
}

std::string WSFrame::GetPayloadText() const {
  std::string payload;
  payload.resize(payload_len_);
  std::memcpy(&payload[0], payload_.data(), payload_len_);
  return payload;
}

std::size_t WSFrame::AppendPayload(const byte_t* data, std::size_t size) {
  if (IsPayloadFull()) {
    return 0;
  }

  std::size_t append_size = std::min(payload_len_ - payload_.size(), size);
  payload_.insert(payload_.end(), data, data + append_size);

  return append_size;
}

std::string WSFrame::Stringify() const {
  std::ostringstream ss;
  ss << *this;
  return ss.str();
}

}  // namespace webcc
