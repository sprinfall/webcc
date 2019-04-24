#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/rest_server.h"
#include "webcc/rest_service.h"

// -----------------------------------------------------------------------------

class FileUploadService : public webcc::RestService {
public:
  void Handle(const webcc::RestRequest& request,
              webcc::RestResponse* response) override {
    if (request.http->method() == "POST") {
      std::cout << "files: " << request.http->form_parts().size() << std::endl;

      for (auto& part : request.http->form_parts()) {
        std::cout << "name: " << part->name() << std::endl;
        std::cout << "data: " << std::endl << part->data() << std::endl;
      }

      response->content = "OK";
      response->media_type = webcc::media_types::kTextPlain;
      response->charset = "utf-8";
      response->status = webcc::Status::kCreated;
    }
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
    webcc::RestServer server(port, workers);

    server.Bind(std::make_shared<FileUploadService>(), "/upload", false);

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
