#include "webcc/ws_client.h"

#include "openssl/sha.h"

#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/endian/conversion.hpp"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/request_builder.h"

using namespace std::placeholders;

namespace webcc {

WSClient::WSClient(boost::asio::io_context& io_context)
    : AsyncClientBase(io_context, "80"), socket_(io_context) {
  buffer_.resize(buffer_size_);
}

void WSClient::Connect(const Url& url, ConnectHandler&& connect_handler) {
  url_ = url;
  connect_handler_ = std::move(connect_handler);

  AsyncHandshake();
}

void WSClient::Send(WSFramePtr frame) {
  LOG_VERB("Send frame:\n%s", frame->Stringify().c_str());

  AsyncWriteFrame(frame);
}

void WSClient::SendPing() {
  auto frame = std::make_shared<WSFrame>();
  frame->Build(ws::opcodes::kPing, ws::NewMaskingKey());
  Send(frame);
}

void WSClient::SendPong() {
  auto frame = std::make_shared<WSFrame>();
  frame->Build(ws::opcodes::kPong, ws::NewMaskingKey());
  Send(frame);
}

void WSClient::SendClose(std::uint16_t code) {
  auto frame = std::make_shared<WSFrame>();

  std::uint32_t masking_key = ws::NewMaskingKey();

  if (code > 0) {
    byte_t data[2];
    ws::HostToNetwork(code, data);
    frame->Build(true, ws::opcodes::kConnectionClose, data, 2, &masking_key);
  } else {
    frame->Build(ws::opcodes::kConnectionClose, masking_key);
  }

  Send(frame);
}

void WSClient::AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                          RWHandler&& handler) {
  boost::asio::async_write(socket_, buffers, std::move(handler));
}

void WSClient::AsyncReadSome(boost::asio::mutable_buffer buffer,
                             RWHandler&& handler) {
  socket_.async_read_some(buffer, std::move(handler));
}

// Check the handshake response.
void WSClient::RequestEnd() {
  do {
    if (error_) {
      LOG_ERRO("WebSocket handshake error");
      break;
    }

    if (response_->status() != status_codes::kSwitchingProtocols) {
      LOG_ERRO("Handshake response status error (%d)", response_->status());
      error_.Set(Error::kHandshakeError,
                 "WebSocket handshake: unexpected status code");
      break;
    }

    // Verify the Sec-WebSocket-Accept header.

    // Generate the expected base64 encoded SHA1 hash.
    std::string key = std::string(request_->GetHeader("Sec-WebSocket-Key"));
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char digest[20];
    SHA1((const unsigned char*)&key[0], key.size(), digest);
    std::string base64_sha1_hash = base64::Encode(&digest[0], 20);

    std::string_view accept = response_->GetHeader("Sec-WebSocket-Accept");
    if (accept != base64_sha1_hash) {
      error_.Set(Error::kHandshakeError,
                 "WebSocket handshake: invalid Sec-WebSocket-Accept header");
      break;
    }

  } while (false);

  if (connect_handler_) {
    connect_handler_(shared_from_this(), error_);
  }

  if (error_) {
    return;
  }

  LOG_INFO("Handshake OK");

  LOG_INFO("Ready to receive messages");
  AsyncReadFrame();
}

void WSClient::HandleFrameIn(WSFramePtr in_frame) {
  if (receive_handler_) {
    receive_handler_(shared_from_this(), in_frame);
  }

  if (in_frame->masked()) {
    LOG_ERRO("Invalid frame received");  // TODO

    // Send a Close frame. (TODO: Close directly? check RFC)
    // TODO: Status code
    SendClose();

    return;
  }

  // TODO
  if (!in_frame->IsPayloadFull()) {
    SendClose();
    return;
  }

  switch (in_frame->opcode()) {
    case ws::opcodes::kPing:
      // Send a Pong in response.
      SendPong();
      break;

    case ws::opcodes::kConnectionClose: {
      close_frame_received_ = true;

      if (close_frame_sent_) {
        // Close the underlying TCP connection.
        CloseSocket();
      } else {
        // Send a Close frame in response.
        std::uint16_t code = 0;
        if (in_frame->payload_len() >= 2) {
          code = ws::NetworkToHost<std::uint16_t>(in_frame->payload());
          LOG_INFO("Close status code: %u", code);
        }
        SendClose(code);
      }

      break;
    }

    default:
      break;
  }
}

void WSClient::AsyncHandshake() {
  Url handshake_url = url_;
  handshake_url.ClearPath();
  handshake_url.AppendPath("chat");

  RequestBuilder builder;
  builder.Method(methods::kGet).Url(std::move(handshake_url));
  builder.Header("Upgrade", "websocket");
  builder.Header("Connection", "Upgrade");
  builder.Header("Sec-WebSocket-Key", base64::Encode(RandomAsciiString(16)));
  builder.Header("Sec-WebSocket-Protocol", "chat, superchat");
  builder.Header("Sec-WebSocket-Version", "13");

  auto request = builder();
  request->Prepare();

  AsyncSend(request, false);
}

void WSClient::AsyncReadFrame() {
  AsyncReadSome(boost::asio::buffer(buffer_),
                std::bind(&WSClient::OnReadFrame, shared_from_this(), _1, _2));
}

void WSClient::OnReadFrame(boost::system::error_code ec, std::size_t size) {
  if (ec) {
    return;
  }

  LOG_INFO("Bytes read: %u", size);

  const byte_t* data = buffer_.data();

  while (true) {
    if (!in_frame_) {
      in_frame_.reset(new WSFrame{});
      parser_.Init(in_frame_.get());
    }

    // Parse the piece of data just read.
    if (!parser_.Parse(data, size)) {
      LOG_ERRO("Failed to parse the frame data");
      CloseSocket();
      error_.Set(Error::kParseError, "Frame data parse error");
      return;
    }

    if (parser_.finished()) {
      LOG_VERB("Frame received:\n%s", in_frame_->Stringify().c_str());

      HandleFrameIn(in_frame_);

      in_frame_.reset();

      if (!parser_.buffer_empty()) {
        // Continue to parse the buffer insider the parser.
        data = nullptr;
        continue;
      }
    }

    break;
  }

  // Continue to read the data frame.
  AsyncReadFrame();
}

void WSClient::AsyncWriteFrame(WSFramePtr frame) {
  if (close_frame_sent_) {
    // Cannot send any more data frames after sending a Close frame.
    LOG_ERRO("A Close frame has been sent");
    // TODO: Call OnError()
    return;
  }

  out_frame_ = frame;

  auto payload = out_frame_->GetPayload();

  AsyncWrite(payload,
             std::bind(&WSClient::OnWriteFrame, shared_from_this(), _1, _2));
}

void WSClient::OnWriteFrame(boost::system::error_code ec, std::size_t length) {
  if (ec) {
    LOG_ERRO("Write frame error (%s)", ec.message().c_str());
    return;
  }

  LOG_VERB("Frame sent:\n%s", out_frame_->Stringify().c_str());

  if (out_frame_->opcode() == ws::opcodes::kConnectionClose) {
    close_frame_sent_ = true;

    if (close_frame_received_) {
      // Close the underlying TCP connection.
      Close();
    }
  }

  out_frame_.reset();
}

}  // namespace webccz