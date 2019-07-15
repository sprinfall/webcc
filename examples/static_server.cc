// A general HTTP server serving static files.

#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/server.h"

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port> <doc_root>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080 D:\\www" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));
  std::string doc_root = argv[2];

  try {
    webcc::Server server(port, doc_root);

    server.Start();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
