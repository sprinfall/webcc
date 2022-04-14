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

WSFrame::WSFrame(const WSFrame& rhs)
    : header_(rhs.header_),
      payload_len_bits_(rhs.payload_len_bits_),
      payload_len_(rhs.payload_len_),
      payload_(rhs.payload_),
      payload_masked_(rhs.payload_masked_) {
}

WSFrame& WSFrame::operator=(const WSFrame& rhs) {
  if (&rhs != this) {
    header_ = rhs.header_;
    payload_len_bits_ = rhs.payload_len_bits_;
    payload_len_ = rhs.payload_len_;
    payload_ = rhs.payload_;
    payload_masked_ = rhs.payload_masked_;
  }
  return *this;
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

void WSFrame::RemoveMask() {
  if (!masked()) {
    return;
  }

  // TODO
}

std::string WSFrame::GetPayloadText() const {
  std::string payload;
  payload.resize(payload_len_);
  std::memcpy(&payload[0], payload_.data(), payload_len_);
  return payload;
}

void WSFrame::Build(bool fin, std::string_view text,
                    std::uint32_t masking_key) {
  const byte_t* data = reinterpret_cast<const byte_t*>(text.data());
  Build(fin, data, text.size(), &masking_key);
}

void WSFrame::Build(bool fin, std::string_view text) {
  const byte_t* data = reinterpret_cast<const byte_t*>(text.data());
  Build(fin, data, text.size(), nullptr);
}

void WSFrame::Build(byte_t opcode) {
  Build(true, nullptr, 0, nullptr);
}

void WSFrame::Build(byte_t opcode, std::uint32_t masking_key) {
  Build(true, nullptr, 0, &masking_key);
}

bool WSFrame::ParseHeader(const byte_t* data, std::size_t size) {
  if (size < 2) {
    return false;
  }

  std::size_t header_size = 2;

  std::uint8_t byte1 = data[1];
  std::uint8_t payload_len7 = byte1 & 0x7F;

  // TODO: Handle payload_len7 == 0
  if (payload_len7 < 126) {
    payload_len_bits_ = ws::PayloadLenBits::k7;
    payload_len_ = payload_len7;

  } else if (payload_len7 == 126) {
    payload_len_bits_ = ws::PayloadLenBits::k16;
    header_size += 2;

    if (size < header_size) {
      return false;
    }

    std::uint16_t len16 = 0;
    std::memcpy(&len16, &data[2], 2);  // TODO: Byte order
    payload_len_ = len16;

  } else if (payload_len7 == 127) {
    payload_len_bits_ = ws::PayloadLenBits::k63;
    header_size += 8;

    if (size < header_size) {
      return false;
    }

    std::uint64_t len64 = 0;
    std::memcpy(&len64, &data[2], 8);  // TODO: Byte order

    if (len64 > ws::kMaxLength63) {
      // TODO: Error handling
    }

    payload_len_ = len64;
  }

  const bool masked = (data[1] >> 7) == 1;

  if (masked) {
    payload_masked_ = true;
    header_size += 4;

    if (size < header_size) {
      return false;
    }
  }

  header_.resize(header_size);
  std::memcpy(&header_[0], data, header_size);

  return true;
}

std::size_t WSFrame::AppendPayload(const byte_t* data, std::size_t size) {
  if (IsPayloadFull()) {
    return 0;
  }

  std::size_t append_size = std::min(payload_len_ - payload_.size(), size);
  payload_.insert(payload_.end(), data, data + append_size);

  return append_size;
}

void WSFrame::Dump(std::ostream& os) const {
  os << "fin: " << static_cast<int>(fin()) << std::endl;
  os << "rsv1~3: " << static_cast<int>(rsv1()) << ","
     << static_cast<int>(rsv2()) << "," << static_cast<int>(rsv3())
     << std::endl;
  os << "opcode: " << static_cast<int>(opcode()) << " ("
     << ws::OpCodeName(opcode()) << ")" << std::endl;

  os << "mask: " << static_cast<int>(mask()) << std::endl;

  os << "payload length: " << payload_len_ << std::endl;
}

std::string WSFrame::Dump() const {
  std::ostringstream ss;
  Dump(ss);
  return ss.str();
}

bool WSFrame::Init(const byte_t* bytes, std::size_t size, bool auto_unmask) {
  if (!ParseHeader(bytes, size)) {
    return false;
  }

  payload_len_ = size - header_.size();
  payload_.resize(payload_len_);
  std::memcpy(&payload_[0], bytes + header_.size(), payload_len_);

  if (masked() && auto_unmask) {
    UnmaskPayload();
  }

  return true;
}

void WSFrame::Build(bool fin, const byte_t* data, std::size_t size,
                    const std::uint32_t* masking_key) {
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
  set_opcode(ws::opcodes::kTextFrame);

  if (payload_len_bits_ == ws::PayloadLenBits::k7) {
    header_[1] = static_cast<byte_t>(payload_len_);

  } else if (payload_len_bits_ == ws::PayloadLenBits::k16) {
    header_[1] = byte_t(126);

    std::uint16_t len = static_cast<std::uint16_t>(payload_len_);
    std::memcpy(&header_[2], &len, 2);

    off += 2;

  } else if (payload_len_bits_ == ws::PayloadLenBits::k63) {
    header_[1] = byte_t(127);

    std::uint64_t len = static_cast<std::uint64_t>(payload_len_);
    std::memcpy(&header_[2], &len, 8);

    off += 8;
  }

  if (masking_key != nullptr) {
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

}  // namespace webcc
