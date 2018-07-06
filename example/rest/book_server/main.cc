#include <iostream>

#include "webcc/logger.h"
#include "webcc/rest_server.h"

#include "book_services.h"

// In order to run with VLD, please copy the following files to the example
// output folder from "third_party\win32\bin":
//   - dbghelp.dll
//   - Microsoft.DTfW.DHL.manifest
//   - vld_x86.dll
#if (defined(WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port> [seconds]" << std::endl;
  std::cout << "If |seconds| is provided, the server will sleep these seconds "
               "before sending back each response."
            << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
  std::cout << "    " << argv0 << " 8080 3" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  int sleep_seconds = 0;
  if (argc >= 3) {
    sleep_seconds = std::atoi(argv[2]);
  }

  std::size_t workers = 2;

  try {
    webcc::RestServer server(port, workers);

    server.Bind(std::make_shared<BookListService>(sleep_seconds),
                "/books", false);

    server.Bind(std::make_shared<BookDetailService>(sleep_seconds),
                "/book/(\\d+)", true);

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
