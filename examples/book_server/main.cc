#include <iostream>

#include "webcc/fs.h"
#include "webcc/logger.h"
#include "webcc/server.h"

#include "views.h"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "usage: book_server <port> <upload_dir>" << std::endl;
    std::cout << "e.g.," << std::endl;
    std::cout << "  $ book_server 8080 D:/upload" << std::endl;
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  webcc::fs::path upload_dir = argv[2];
  if (!webcc::fs::is_directory(upload_dir) || !webcc::fs::exists(upload_dir)) {
    std::cerr << "Invalid upload dir!" << std::endl;
    return 1;
  }

  // Add a sub-dir for book photos.
  webcc::fs::path photo_dir = upload_dir / "books";
  if (!webcc::fs::exists(photo_dir)) {
    webcc::fs::create_directory(photo_dir);
  }

  std::cout << "Book photos will be saved to: " << photo_dir << std::endl;

  try {
    webcc::Server server{ boost::asio::ip::tcp::v4(), port };

    server.Route("/books",
                 std::make_shared<BookListView>(),
                 { "GET", "POST" });

    server.Route(webcc::R("/books/(\\d+)"),
                 std::make_shared<BookDetailView>(photo_dir),
                 { "GET", "PUT", "DELETE" });

    server.Route(webcc::R("/books/(\\d+)/photo"),
                 std::make_shared<BookPhotoView>(photo_dir),
                 { "GET", "PUT", "DELETE" });

    server.Run(2);

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
