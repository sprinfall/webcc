#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

// -----------------------------------------------------------------------------

class FileUploadView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "POST") {
      return Post(request);
    }

    return {};
  }

private:
  webcc::ResponsePtr Post(webcc::RequestPtr request) {
    std::cout << "form parts: " << request->form_parts().size() << std::endl;

    for (auto& part : request->form_parts()) {
      std::cout << "name: " << part->name() << std::endl;
      std::cout << "data: " << std::endl << part->data() << std::endl;
    }

    return webcc::ResponseBuilder{}.Created().Body("OK")();
  }
};

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  try {
    webcc::Server server(port);

    server.Route("/upload", std::make_shared<FileUploadView>(), { "POST" });

    server.Start();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
