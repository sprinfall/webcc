// examples/hello_server.cc
// A simple HTTP server which returns "Hello, World!" to the client.
// This example is also used as the test server for some client examples (e.g.,
// heartbeat_client).

#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

class HelloView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    int seconds = 0;
    if (request->args().size() == 1) {
      seconds = std::stoi(request->args()[0]);
    }

    if (seconds > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }

    if (request->method() == "GET") {
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }
};

int main(int argc, const char* argv[]) {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  int workers = 1;
  int loops = 1;

  if (argc > 1) {
    workers = std::stoi(argv[1]);
    if (argc > 2) {
      loops = std::stoi(argv[2]);
    }
  }

  LOG_USER("Workers: %d, loops: %d", workers, loops);

  try {
    webcc::Server server{ boost::asio::ip::tcp::v4(), 8080 };

    auto view = std::make_shared<HelloView>();

    server.Route("/", view);
    server.Route("/hello", view);
    server.Route(webcc::R{ "/sleep/(\\d+)" }, view);

    server.Run(workers, loops);

  } catch (const std::exception&) {
    return 1;
  }

  return 0;
}
