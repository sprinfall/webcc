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
    webcc::Server server(8080);

    server.Route("/", std::make_shared<HelloView>());

    server.Start();

  } catch (const std::exception&) {
    return 1;
  }

  return 0;
}
