#include "webcc/ws_parser.h"

#include "webcc/logger.h"

namespace webcc {

bool WSParser::Parse(const byte_t* data, std::size_t size) {
  if (header_parsed_) {
    if (data != nullptr) {
      AppendPayload(data, size);
    }
    return true;
  }

  if (data != nullptr) {
    buffer_.insert(buffer_.end(), data, data + size);
  }

  if (!ParseHeader()) {
    return false;
  }

  if (!header_parsed_) {
    LOG_INFO("Frame header will continue in next read");
    return true;
  }

  LOG_INFO("Frame header just ended");

  // The left data, if any, is still in the buffer.
  if (!buffer_.empty()) {
    std::size_t append_size = AppendPayload(buffer_.data(), buffer_.size());
    buffer_.erase(buffer_.begin(), buffer_.begin() + append_size);
  }

  return true;
}

bool WSParser::ParseHeader() {
  if (!frame_->ParseHeader(buffer_.data(), buffer_.size())) {
    return true;
  }

  // Remove the data which has just been parsed.
  buffer_.erase(buffer_.begin(), buffer_.begin() + frame_->header_len());

  header_parsed_ = true;
  return true;
}

std::size_t WSParser::AppendPayload(const byte_t* data, std::size_t size) {
  std::size_t append_size = frame_->AppendPayload(data, size);

  if (frame_->IsPayloadFull()) {
    finished_ = true;
  }

  return append_size;
}

}  // namespace webcc
