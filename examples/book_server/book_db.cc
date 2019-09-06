#include "book_db.h"

#include <algorithm>

const Book& BookDB::Get(const std::string& id) const {
  auto it = Find(id);
  return (it == books_.end() ? kNullBook : *it);
}

std::string BookDB::Add(const Book& book) {
  std::string id = NewID();
  books_.push_back({ id, book.title, book.price });
  return id;
}

bool BookDB::Set(const Book& book) {
  auto it = Find(book.id);
  if (it != books_.end()) {
    it->title = book.title;
    it->price = book.price;
    return true;
  }
  return false;
}

std::string BookDB::GetPhoto(const std::string& id) const {
  auto it = Find(id);
  return it != books_.end() ? it->photo : "";
}

bool BookDB::SetPhoto(const std::string& id, const std::string& photo) {
  auto it = Find(id);
  if (it != books_.end()) {
    it->photo = photo;
    return true;
  }
  return false;
}

bool BookDB::Delete(const std::string& id) {
  auto it = Find(id);

  if (it != books_.end()) {
    books_.erase(it);
    return true;
  }

  return false;
}

std::list<Book>::const_iterator BookDB::Find(const std::string& id) const {
  return std::find_if(books_.begin(), books_.end(),
                      [&id](const Book& book) { return book.id == id; });
}

std::list<Book>::iterator BookDB::Find(const std::string& id) {
  return std::find_if(books_.begin(), books_.end(),
                      [&id](Book& book) { return book.id == id; });
}

std::string BookDB::NewID() const {
  static int s_id_counter = 0;

  ++s_id_counter;
  return std::to_string(s_id_counter);
}
