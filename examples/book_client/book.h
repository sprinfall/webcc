#ifndef BOOK_H_
#define BOOK_H_

#include <iosfwd>
#include <string>

struct Book {
  std::string id;
  std::string title;
  double price;
  std::string photo;  // Name only

  bool IsNull() const {
    return id.empty();
  }
};

std::ostream& operator<<(std::ostream& os, const Book& book);

extern const Book kNullBook;

#endif  // BOOK_H_
