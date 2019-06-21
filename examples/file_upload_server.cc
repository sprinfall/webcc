#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/server.h"

// -----------------------------------------------------------------------------

class FileUploadService : public webcc::Service {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request,
                            const webcc::UrlArgs& args) override {
    if (request->method() == "POST") {
      std::cout << "files: " << request->form_parts().size() << std::endl;

      for (auto& part : request->form_parts()) {
        std::cout << "name: " << part->name() << std::endl;
        std::cout << "data: " << std::endl << part->data() << std::endl;
      }

      // TODO: media_type: webcc::media_types::kTextPlain; charset = "utf-8";
      return webcc::ResponseBuilder{}.Created().Data("OK")();
    }

    return webcc::ResponseBuilder{}.NotImplemented()();
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

  std::size_t workers = 2;

  try {
    // TODO: doc root
    webcc::Server server(port, workers);

    server.Bind(std::make_shared<FileUploadService>(), "/upload", false);

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
