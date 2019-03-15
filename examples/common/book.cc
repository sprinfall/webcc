#include "examples/common/book.h"

#include <algorithm>
#include <iostream>

const Book kNullBook{};

std::ostream& operator<<(std::ostream& os, const Book& book) {
  os << "{ " << book.id << ", " << book.title << ", " << book.price << " }";
  return os;
}

const Book& BookStore::GetBook(const std::string& id) const {
  auto it = FindBook(id);
  return (it == books_.end() ? kNullBook : *it);
}

std::string BookStore::AddBook(const Book& book) {
  std::string id = NewID();
  books_.push_back({ id, book.title, book.price });
  return id;
}

bool BookStore::UpdateBook(const Book& book) {
  auto it = FindBook(book.id);
  if (it != books_.end()) {
    it->title = book.title;
    it->price = book.price;
    return true;
  }
  return false;
}

bool BookStore::DeleteBook(const std::string& id) {
  auto it = FindBook(id);

  if (it != books_.end()) {
    books_.erase(it);
    return true;
  }

  return false;
}

std::list<Book>::const_iterator BookStore::FindBook(const std::string& id)
    const {
  return std::find_if(books_.begin(), books_.end(),
                      [&id](const Book& book) { return book.id == id; });
}

std::list<Book>::iterator BookStore::FindBook(const std::string& id) {
  return std::find_if(books_.begin(), books_.end(),
                      [&id](Book& book) { return book.id == id; });
}

std::string BookStore::NewID() const {
  static int s_id_counter = 0;

  ++s_id_counter;
  return std::to_string(s_id_counter);
}
