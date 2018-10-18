#include <iostream>

#include "webcc/logger.h"

#include "example/soap_book_client/book_client.h"

#if (defined(WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " localhost 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::string host = argv[1];
  std::string port = argv[2];

  BookClient client(host, port);

  std::string id1;
  if (!client.CreateBook("1984", 12.3, &id1)) {
    std::cerr << "Failed to create book." << std::endl;
    return 2;
  }

  std::cout << "Book ID: " << id1 << std::endl;

  std::string id2;
  if (!client.CreateBook("1Q84", 32.1, &id2)) {
    std::cerr << "Failed to create book." << std::endl;
    return 2;
  }

  std::cout << "Book ID: " << id2 << std::endl;

  Book book;
  if (!client.GetBook(id1, &book)) {
    std::cerr << "Failed to get book." << std::endl;
    return 2;
  }

  std::cout << "Book: " << book << std::endl;

  std::list<Book> books;
  if (!client.ListBooks(&books)) {
    std::cerr << "Failed to list books." << std::endl;
    return 2;
  }

  for (const Book& book : books) {
    std::cout << "Book: " << book << std::endl;
  }

  if (client.DeleteBook(id1)) {
    std::cout << "Book deleted: " << id1 << std::endl;
  }

  return 0;
}
