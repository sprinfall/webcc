#include "book.h"

#include <iostream>

const Book kNullBook{};

std::ostream& operator<<(std::ostream& os, const Book& book) {
  os << "{ " << book.id << ", " << book.title << ", " << book.price << ", "
     << book.photo << " }";
  return os;
}
