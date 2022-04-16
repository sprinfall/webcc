#include <iostream>

#include "webcc/logger.h"
#include "webcc/ws_client.h"

struct ConnectHandler {
  void operator()(webcc::WSClientPtr ws_client, const webcc::Error& error) {
    if (error) {
      std::cerr << "Connect error: " << error.what() << std::endl;
      return;
    }

    std::cout << "Send 'Hello'" << std::endl;

    auto frame = std::make_shared<webcc::WSFrame>();
    frame->Build(true, "Hello", webcc::ws::NewMaskingKey());

    ws_client->Send(frame);
  }
};

struct ReceiveHandler {
  void operator()(webcc::WSClientPtr ws_client, webcc::WSFramePtr frame) {
    if (frame->opcode() == webcc::ws::opcodes::kTextFrame) {
      // Close connection
      ws_client->SendClose(webcc::ws::status_codes::kNormalClosure);
    }
  }
};

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  boost::asio::io_context io_context;

  webcc::Url url{ "ws://localhost:8080" };

  auto ws_client = webcc::WSClient::Make(io_context);

  ws_client->set_receive_handler(ReceiveHandler{});
  ws_client->Connect(url, ConnectHandler{});

  io_context.run();

  return 0;
}
