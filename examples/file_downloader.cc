// Download files.
// This example demonstrates the usage of streamed response.

#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "usage: file_downloader <url> <path>" << std::endl;
    std::cout << std::endl;
    std::cout << "examples:" << std::endl;
    std::cout << "  $ file_downloader http://httpbin.org/image/jpeg D:/test.jpg"
              << std::endl;
    std::cout << "  $ file_downloader https://www.google.com/favicon.ico"
              << " D:/test.ico" << std::endl;
    return 1;
  }

  const char* url = argv[1];
  const char* path = argv[2];

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  try {
    auto r = session.Request(webcc::RequestBuilder{}.Get(url)(),
                             true);  // Stream the response data to file.

    if (auto file_body = r->file_body()) {
      file_body->Move(path);
    }

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
