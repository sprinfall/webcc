#include "webcc/ws_client.h"

#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"

#include "webcc/logger.h"
#include "webcc/request_builder.h"

using namespace std::placeholders;

namespace webcc {

WSClient::WSClient(boost::asio::io_context& io_context)
    : AsyncClientBase(io_context, "80"), socket_(io_context) {
  buffer_.resize(buffer_size_);  // TODO
}

void WSClient::Connect(const Url& url, ConnectHandler&& connect_handler) {
  url_ = url;
  connect_handler_ = std::move(connect_handler);

  AsyncHandshake();
}

void WSClient::Send(WSFramePtr frame) {
  AsyncWriteFrame(frame);
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
      LOG_ERRO("Handshake error");
      break;
    }

    if (response_->status() != status_codes::kSwitchingProtocols) {
      LOG_ERRO("Handshake response status error (%d)", response_->status());
      error_.Set(Error::kSyntaxError,  // TODO: Define a new error code
                 "Websocket handshake: unexpected status code");
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

// TODO: Use `in_frame_` directly?
void WSClient::HandleFrameIn(WSFramePtr in_frame) {
  if (receive_handler_) {
    receive_handler_(shared_from_this(), in_frame);
  }

  if (in_frame->masked()) {
    LOG_ERRO("Invalid frame received");  // TODO

    // Send a Close frame. (TODO: Close directly? check RFC)
    // TODO: Status code
    auto close_frame = std::make_shared<WSFrame>();
    close_frame->Build(ws::opcodes::kConnectionClose, ws::NewMaskingKey());

    AsyncWriteFrame(close_frame);

    return;
  }

  const byte_t opcode = in_frame->opcode();

  switch (opcode) {
    case ws::opcodes::kPing: {
      //  Send a Pong frame in response.
      // Reuse the incoming frame instead of creating a new one.
      in_frame->set_opcode(ws::opcodes::kPong);
      in_frame->AddMask(ws::NewMaskingKey());
      AsyncWriteFrame(in_frame);

      break;
    }

    case ws::opcodes::kConnectionClose: {
      close_frame_received_ = true;

      if (close_frame_sent_) {
        // Close the underlying TCP connection.
        Close();
      } else {
        // Send a Close frame in response.
        // TODO: Parse status code
        in_frame->AddMask(ws::NewMaskingKey());
        AsyncWriteFrame(in_frame);
      }

      break;
    }

    default:
      // TODO
      break;
  }
}

void WSClient::AsyncHandshake() {
  Url handshake_url = url_;
  handshake_url.ClearPath();
  handshake_url.AppendPath("chat");

  RequestBuilder request_builder;
  request_builder.Method(methods::kGet).Url(std::move(handshake_url));
  request_builder.Header("Upgrade", "websocket");
  request_builder.Header("Connection", "Upgrade");
  request_builder.Header("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
  request_builder.Header("Sec-WebSocket-Protocol", "chat, superchat");
  request_builder.Header("Sec-WebSocket-Version", "13");

  auto request = request_builder();

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
      LOG_VERB("Frame received:\n%s", in_frame_->Dump().c_str());

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

  LOG_VERB("Frame sent:\n%s", out_frame_->Dump().c_str());

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