#include "gtest/gtest.h"

#include <iostream>

#include "webcc/ws_frame.h"

using webcc::byte_t;
using webcc::ws::PayloadLenBits;
namespace opcodes = webcc::ws::opcodes;

TEST(WSFrame, UnmaskedTextLen7_Parse) {
  // A single-frame unmasked text message (contains "Hello")
  const byte_t frame_data[] = { 0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };

  webcc::WSFrame frame{ frame_data, sizeof(frame_data) };

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

TEST(WSFrame, UnmaskedTextLen7_Build) {
  webcc::WSFrame frame;
  frame.Build(true, "Hello");

  EXPECT_EQ(frame.size(), 2 + 5);

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

TEST(WSFrame, MaskedTextLen7_Parse) {
  // A single-frame masked text message (contains "Hello")
  const byte_t frame_data[] = { 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d,
                                0x7f, 0x9f, 0x4d, 0x51, 0x58 };

  webcc::WSFrame frame{ frame_data, sizeof(frame_data) };

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame.mask(), 1);
  EXPECT_TRUE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame.payload_len(), 5);
  EXPECT_EQ(frame.GetPayloadText(), "Hello");
}

TEST(WSFrame, MaskedTextLen7_Build) {
  webcc::WSFrame frame;
  frame.Build(true, "Hello", webcc::ws::NewMaskingKey());

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kTextFrame);

  EXPECT_EQ(frame.mask(), 1);
  EXPECT_TRUE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k7);
  EXPECT_EQ(frame.payload_len(), 5);

  EXPECT_TRUE(frame.payload_masked());

  frame.UnmaskPayload();
  EXPECT_EQ(frame.GetPayloadText(), "Hello");
}

TEST(WSFrame, UnmaskedBinaryLen16_Parse) {
  // 256 bytes binary message in a single unmasked frame
  const byte_t frame_data[4 + 256] = { 0x82, 0x7E, 0x00, 0x01 };

  webcc::WSFrame frame{ frame_data, sizeof(frame_data) };

  EXPECT_EQ(frame.fin(), 1);
  EXPECT_EQ(frame.rsv1(), 0);
  EXPECT_EQ(frame.rsv2(), 0);
  EXPECT_EQ(frame.rsv3(), 0);
  EXPECT_EQ(frame.opcode(), opcodes::kBinaryFrame);

  EXPECT_EQ(frame.mask(), 0);
  EXPECT_FALSE(frame.masked());

  EXPECT_EQ(frame.payload_len_bits(), PayloadLenBits::k16);
  EXPECT_EQ(frame.payload_len(), 256);
}
