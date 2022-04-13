#ifndef WEBCC_WS_PARSER_H_
#define WEBCC_WS_PARSER_H_

#include <vector>

#include "webcc/common.h"
#include "webcc/globals.h"
#include "webcc/ws_frame.h"

namespace webcc {

class WSParser {
public:
  WSParser() = default;

  WSParser(const WSParser&) = delete;
  WSParser& operator=(const WSParser&) = delete;

  ~WSParser() = default;

  void Init(WSFrame* frame) {
    frame_ = frame;
    header_parsed_ = false;
    finished_ = false;
  }

  bool header_parsed() const {
    return header_parsed_;
  }

  bool buffer_empty() const {
    return buffer_.empty();
  }

  bool finished() const {
    return finished_;
  }

  // Parse the given length of data.
  // Return false if the parsing is failed.
  bool Parse(const byte_t* data, std::size_t size);

private:
  bool ParseHeader();

  std::size_t AppendPayload(const byte_t* data, std::size_t size);

  WSFrame* frame_ = nullptr;

  std::vector<byte_t> buffer_;

  bool header_parsed_ = false;
  bool finished_ = false;
};

}  // namespace webcc

#endif  // WEBCC_WS_PARSER_H_
