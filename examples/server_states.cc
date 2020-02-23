#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

class HelloView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }
};

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  try {
    webcc::Server server{ 8080 };

    server.Route("/", std::make_shared<HelloView>());

    // Run the server in a separate thread.
    std::thread t([&server]() { server.Run(); });

    // Let the server run for several seconds.
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Stop the server.
    server.Stop();

    // Wait for the server to finish.
    t.join();

    // Run the server again.
    std::thread t2([&server]() { server.Run(); });

    // Wait for the server to finish.
    t2.join();

  } catch (const std::exception&) {
    // NOTE:
    // Catch std::exception instead of webcc::Error.
    // webcc::Error is for client only. 
    return 1;
  }

  return 0;
}
