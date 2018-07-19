#ifndef EXAMPLE_COMMON_BOOK_H_
#define EXAMPLE_COMMON_BOOK_H_

#include <list>
#include <string>

// In-memory test data.
// There should be some database in a real product.

struct Book {
  std::string id;
  std::string title;
  double price;

  bool IsNull() const { return id.empty(); }
};

std::ostream& operator<<(std::ostream& os, const Book& book);

extern const Book kNullBook;

class BookStore {
 public:
  const std::list<Book>& books() const { return books_; }

  const Book& GetBook(const std::string& id) const;

  // Add a book, return the ID.
  // NOTE: The ID of the input book will be ignored so should be empty.
  std::string AddBook(const Book& book);

  bool UpdateBook(const Book& book);

  bool DeleteBook(const std::string& id);

 private:
  std::list<Book>::const_iterator FindBook(const std::string& id) const;

  std::list<Book>::iterator FindBook(const std::string& id);

  // Allocate a new book ID.
  std::string NewID() const;

  std::list<Book> books_;
};

#endif  // EXAMPLE_COMMON_BOOK_H_
