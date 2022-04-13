#include "gtest/gtest.h"

#include <iostream>

#include "webcc/ws_frame.h"
#include "webcc/ws_parser.h"

using webcc::byte_t;
using webcc::ws::PayloadLenBits;
namespace opcodes = webcc::ws::opcodes;

TEST(WSParser, ParseInOneStep) {
  // A single-frame unmasked text message (contains "Hello")
  const byte_t frame_data[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };

  webcc::WSFrame frame;

  webcc::WSParser parser;
  parser.Init(&frame);

  bool result = parser.Parse(frame_data, sizeof(frame_data));
  EXPECT_TRUE(result);

  EXPECT_TRUE(parser.finished());
  EXPECT_TRUE(parser.buffer_empty());

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame.mask(), 0);
  EXPECT_FALSE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame.payload_len(), 5);
  EXPECT_EQ(frame.GetPayloadText(), "Hello");
}

TEST(WSParser, ParseInTwoSteps) {
  // A single-frame unmasked text message (contains "Hello")
  const byte_t frame_data[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };

  webcc::WSFrame frame;

  webcc::WSParser parser;
  parser.Init(&frame);

  // Step 1: 2 bytes

  bool result = parser.Parse(frame_data, 2);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.buffer_empty());
  EXPECT_FALSE(parser.finished());

  // Step 2: left bytes

  result = parser.Parse(frame_data + 2, sizeof(frame_data) - 2);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.buffer_empty());
  EXPECT_TRUE(parser.finished());

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame.mask(), 0);
  EXPECT_FALSE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame.payload_len(), 5);
  EXPECT_EQ(frame.GetPayloadText(), "Hello");
}

TEST(WSParser, ParseInThreeSteps) {
  // A single-frame unmasked text message (contains "Hello")
  const byte_t frame_data[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };

  webcc::WSFrame frame;

  webcc::WSParser parser;
  parser.Init(&frame);

  // Step 1: 1 byte

  bool result = parser.Parse(frame_data, 1);

  EXPECT_TRUE(result);
  EXPECT_FALSE(parser.buffer_empty());
  EXPECT_FALSE(parser.finished());

  // Step 2: 3 bytes

  result = parser.Parse(frame_data + 1, 3);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.buffer_empty());
  EXPECT_FALSE(parser.finished());

  // Step 3: left bytes

  result = parser.Parse(frame_data + 4, sizeof(frame_data) - 4);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.buffer_empty());
  EXPECT_TRUE(parser.finished());

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame.mask(), 0);
  EXPECT_FALSE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame.payload_len(), 5);
  EXPECT_EQ(frame.GetPayloadText(), "Hello");
}

TEST(WSParser, ParseMixedFrames) {
  // clang-format off
  const byte_t frame_data[] = {
    // A single-frame unmasked text message (contains "Hello")
      0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
      // A single-frame masked text message (contains "Hello")
      0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
  };
  // clang-format on

  webcc::WSFrame frame1;

  webcc::WSParser parser;
  parser.Init(&frame1);

  bool result = parser.Parse(frame_data, sizeof(frame_data));

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.finished());
  
  EXPECT_FALSE(parser.buffer_empty());

  EXPECT_EQ(frame1.fin(), 1);
  EXPECT_EQ(frame1.rsv1(), 0);
  EXPECT_EQ(frame1.rsv2(), 0);
  EXPECT_EQ(frame1.rsv3(), 0);
  EXPECT_EQ(frame1.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame1.mask(), 0);
  EXPECT_FALSE(frame1.masked());

  EXPECT_EQ(frame1.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame1.payload_len(), 5);
  EXPECT_EQ(frame1.GetPayloadText(), "Hello");

  webcc::WSFrame frame2;
  parser.Init(&frame2);
  result = parser.Parse(nullptr, 0);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.finished());
  EXPECT_TRUE(parser.buffer_empty());

  EXPECT_EQ(frame2.fin(), 1);
  EXPECT_EQ(frame2.rsv1(), 0);
  EXPECT_EQ(frame2.rsv2(), 0);
  EXPECT_EQ(frame2.rsv3(), 0);
  EXPECT_EQ(frame2.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame2.mask(), 1);
  EXPECT_TRUE(frame2.masked());

  EXPECT_EQ(frame2.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame2.payload_len(), 5);

  frame2.UnmaskPayload();
  EXPECT_EQ(frame2.GetPayloadText(), "Hello");
}

TEST(WSParser, ParseMixedFramesInMultiSteps) {
  // clang-format off
  const byte_t frame_data[] = {
    // A single-frame unmasked text message (contains "Hello")
    0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
    // A single-frame masked text message (contains "Hello")
    0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
  };
  // clang-format on

  webcc::WSFrame frame1;

  webcc::WSParser parser;
  parser.Init(&frame1);

  bool result = parser.Parse(frame_data, 10);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.finished());

  EXPECT_EQ(frame1.fin(), 1);
  EXPECT_EQ(frame1.rsv1(), 0);
  EXPECT_EQ(frame1.rsv2(), 0);
  EXPECT_EQ(frame1.rsv3(), 0);
  EXPECT_EQ(frame1.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame1.mask(), 0);
  EXPECT_FALSE(frame1.masked());

  EXPECT_EQ(frame1.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame1.payload_len(), 5);
  EXPECT_EQ(frame1.GetPayloadText(), "Hello");

  EXPECT_FALSE(parser.buffer_empty());

  webcc::WSFrame frame2;
  parser.Init(&frame2);
  result = parser.Parse(frame_data + 10, sizeof(frame_data) - 10);

  EXPECT_TRUE(result);
  EXPECT_TRUE(parser.finished());
  EXPECT_TRUE(parser.buffer_empty());

  EXPECT_EQ(frame2.fin(), 1);
  EXPECT_EQ(frame2.rsv1(), 0);
  EXPECT_EQ(frame2.rsv2(), 0);
  EXPECT_EQ(frame2.rsv3(), 0);
  EXPECT_EQ(frame2.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame2.mask(), 1);
  EXPECT_TRUE(frame2.masked());

  EXPECT_EQ(frame2.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame2.payload_len(), 5);

  frame2.UnmaskPayload();
  EXPECT_EQ(frame2.GetPayloadText(), "Hello");
}
