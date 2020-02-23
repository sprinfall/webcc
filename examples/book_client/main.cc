#include <filesystem>
#include <iostream>

#include "webcc/logger.h"

#include "book_client.h"

// -----------------------------------------------------------------------------

void PrintSeparator() {
  static const std::string s_line(80, '-');
  std::cout << s_line << std::endl;
}

void PrintBook(const Book& book) {
  std::cout << "Book: " << book << std::endl;
}

void PrintBookList(const std::list<Book>& books) {
  std::cout << "Book list: " << books.size() << std::endl;
  for (const Book& book : books) {
    std::cout << "  Book: " << book << std::endl;
  }
}

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "usage: book_client <url> <photo_dir>" << std::endl;
    std::cout << "e.g.," << std::endl;
    std::cout << "  $ book_client http://localhost:8080 path/to/photo_dir"
              << std::endl;
    return 1;
  }

  std::string url = argv[1];

  std::filesystem::path photo_dir = argv[2];
  if (!std::filesystem::is_directory(photo_dir) ||
      !std::filesystem::exists(photo_dir)) {
    std::cerr << "Invalid photo dir!" << std::endl;
    return 1;
  }

  std::cout << "Test photo dir: " << photo_dir << std::endl;

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE_FILE_OVERWRITE);

  BookClient client{ url };

  PrintSeparator();

  std::list<Book> books;
  if (client.Query(&books)) {
    PrintBookList(books);
  } else {
    return 1;
  }

  PrintSeparator();

  std::string id;
  if (client.Create("1984", 12.3, &id)) {
    std::cout << "Book ID: " << id << std::endl;
  } else {
    return 1;
  }

  if (!client.SetPhoto(id, photo_dir / "1984.jpg")) {
    return 1;
  }

  PrintSeparator();

  books.clear();
  if (client.Query(&books)) {
    PrintBookList(books);
  } else {
    return 1;
  }

  PrintSeparator();

  Book book;
  if (client.Get(id, &book)) {
    PrintBook(book);
  } else {
    return 1;
  }

  PrintSeparator();

  std::cout << "Press any key to continue...";
  std::getchar();

  if (!client.Set(id, "1Q84", 32.1)) {
    return 1;
  }

  if (!client.SetPhoto(id, photo_dir / "1Q84.jpg")) {
    return 1;
  }

  PrintSeparator();

  if (client.Get(id, &book)) {
    PrintBook(book);
  } else {
    return 1;
  }

  PrintSeparator();

  std::cout << "Press any key to continue...";
  std::getchar();

  if (!client.Delete(id)) {
    return 1;
  }

  PrintSeparator();

  books.clear();
  if (client.Query(&books)) {
    PrintBookList(books);
  }

  std::cout << "Press any key to continue...";
  std::getchar();

  return 0;
}
