// A server handling multipart form data.

#include <fstream>
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

      if (part->file_name().empty()) {
        std::cout << "data: " << part->data() << std::endl;
      } else {
        // Save part->data() as binary to file.
        // ...
      }
    }

    return webcc::ResponseBuilder{}.Created().Body("OK")();
  }
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: form_server <port>" << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  $ form_server 8080" << std::endl;
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  try {
    webcc::Server server{ boost::asio::ip::tcp::v4(), port };

    server.set_buffer_size(webcc::kBufferSize * 10);

    server.Route("/upload", std::make_shared<FileUploadView>(), { "POST" });

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
