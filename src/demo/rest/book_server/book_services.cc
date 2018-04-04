#include "book_services.h"

#include <list>
#include "boost/lexical_cast.hpp"

////////////////////////////////////////////////////////////////////////////////

class Book {
public:
  std::string id;
  std::string title;
  double price;

  bool IsNull() const {
    return id.empty();
  }
};

static const Book kNullBook{};

class BookStore {
public:
  BookStore() {
    books_.push_back({ "1", "Title1", 11.1 });
    books_.push_back({ "2", "Title2", 22.2 });
    books_.push_back({ "3", "Title3", 33.3 });
  }

  const std::list<Book>& books() const {
    return books_;
  }

  const Book& GetBook(const std::string& id) const {
    auto it = std::find_if(books_.begin(),
                           books_.end(),
                           [&id](const Book& book) { return book.id == id; });

    if (it == books_.end()) {
      return kNullBook;
    }

    return *it;
  }

  bool AddBook(const Book& new_book) {
    auto it = std::find_if(
        books_.begin(),
        books_.end(),
        [&new_book](const Book& book) { return book.id == new_book.id; });

    if (it != books_.end()) {
      return false;
    }

    books_.push_back(new_book);
    return true;
  }

  bool DeleteBook(const std::string& id) {
    auto it = std::find_if(books_.begin(),
                           books_.end(),
                           [&id](const Book& book) { return book.id == id; });

    if (it == books_.end()) {
      return false;
    }

    books_.erase(it);
    return true;
  }

private:
  std::list<Book> books_;
};

static BookStore g_book_store;

////////////////////////////////////////////////////////////////////////////////

// Naively create JSON object string for a book.
// You should use real JSON library in your product code.
static std::string CreateBookJson(const Book& book) {
  std::string json = "{ ";
  json += "\"id\": " + book.id + ", ";
  json += "\"title\": " + book.title + ", ";
  json += "\"price\": " + std::to_string(book.price);
  json += " }";
  return json;
}

bool BookListService::Handle(const std::string& http_method,
                             const std::vector<std::string>& url_sub_matches,
                             const std::string& request_content,
                             std::string* response_content) {
  if (http_method == webcc::kHttpGet) {
    *response_content = "{ ";
    for (const Book& book : g_book_store.books()) {
      *response_content += CreateBookJson(book);
      *response_content += ",";
    }
    *response_content += " }";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool BookDetailService::Handle(const std::string& http_method,
                               const std::vector<std::string>& url_sub_matches,
                               const std::string& request_content,
                               std::string* response_content) {
  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  if (http_method == webcc::kHttpGet) {
    const Book& book = g_book_store.GetBook(book_id);

    if (book.IsNull()) {
      return false;
    }

    *response_content = CreateBookJson(book);

    return true;

  } else if (http_method == webcc::kHttpPost) {

  } else if (http_method == webcc::kHttpDelete) {

  }

  return false;
}
