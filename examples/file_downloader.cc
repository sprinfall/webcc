// examples/file_downloader.cc
// This example downloads a JPG file.

// NOTE:
// - The response is streamed to file to avoid comsuming large memory.
// - A callback is set to be informed about the download progress.

#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: file_downloader <url> <path>" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  $ file_downloader http://httpbin.org/image/jpeg ./test.jpg"
              << std::endl;
    std::cout
        << "  $ file_downloader https://www.google.com/favicon.ico ./test.ico"
        << "" << std::endl;
    return 1;
  }

  const char* url = argv[1];
  const char* path = argv[2];

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  // NOTE(20231121): Try the new added `subsequent read timeout`.
  session.set_subsequent_read_timeout(5);

  auto on_progress = [](std::size_t length, std::size_t total_length) {
    if (total_length > 0) {  // Avoid dividing zero
      int percent = static_cast<int>(length * 100.0 / total_length);
      std::cout << "Download progress: " << percent << "%" << std::endl;
    }
  };

  try {
    auto r = session.Send(WEBCC_GET(url)(), true, on_progress);

    if (auto file_body = r->file_body()) {
      file_body->Move(path);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
