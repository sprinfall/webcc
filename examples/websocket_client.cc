#include <iostream>

#include "webcc/logger.h"
#include "webcc/ws_client.h"

namespace wws = webcc::ws;

void ConnectHandler(webcc::WSClientPtr ws_client, webcc::Error error) {
  if (error) {
    std::cerr << "Connect error: " << error.what() << std::endl;
    return;
  }

  std::cout << "Send 'Hello'" << std::endl;

  auto frame = std::make_shared<webcc::WSFrame>();
  frame->Build(true, "Hello", wws::NewMaskingKey());

  ws_client->Send(frame);
}

void ReceiveHandler(webcc::WSClientPtr ws_client, webcc::WSFramePtr frame) {
  if (frame->opcode() == wws::opcodes::kTextFrame) {
    // Close connection
    auto frame = std::make_shared<webcc::WSFrame>();
    frame->Build(wws::opcodes::kConnectionClose, wws::NewMaskingKey());
    ws_client->Send(frame);
  }
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  boost::asio::io_context io_context;

  webcc::Url url{ "ws://localhost:8080" };

  auto ws_client = std::make_shared<webcc::WSClient>(io_context);

  ws_client->set_receive_handler(ReceiveHandler);
  ws_client->Connect(url, ConnectHandler);

  io_context.run();

  return 0;
}
