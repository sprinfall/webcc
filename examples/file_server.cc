// A general HTTP server serving static files.

#include <iostream>
#include <string>

#include "webcc/logger.h"
#include "webcc/server.h"

void Help() {
  std::cout << "Usage:" << std::endl;
  std::cout << "    file_server <port> <doc_root> [chunk_size]" << std::endl;
  std::cout << "E.g.," << std::endl;
  std::cout << "    file_server 8080 D:/www" << std::endl;
  std::cout << "    file_server 8080 D:/www 10000" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    Help();
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));
  std::string doc_root = argv[2];

  try {
    webcc::Server server(port, doc_root);

    if (argc == 4) {
      server.set_file_chunk_size(std::atoi(argv[3]));
    }

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
