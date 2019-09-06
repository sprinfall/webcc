#ifndef BOOK_DB_H_
#define BOOK_DB_H_

#include <list>
#include <string>

#include "book.h"

// Book database simulator.
// There should be some database in a real product.

class BookDB {
public:
  const std::list<Book>& books() const {
    return books_;
  }

  const Book& Get(const std::string& id) const;

  // Add a book, return the ID.
  // NOTE: The ID of the input book will be ignored so should be empty.
  std::string Add(const Book& book);

  bool Set(const Book& book);

  std::string GetPhoto(const std::string& id) const;

  bool SetPhoto(const std::string& id, const std::string& photo);

  bool Delete(const std::string& id);

private:
  std::list<Book>::const_iterator Find(const std::string& id) const;

  std::list<Book>::iterator Find(const std::string& id);

  // Allocate a new book ID.
  std::string NewID() const;

private:
  std::list<Book> books_;
};

#endif  // BOOK_DB_H_
