#include "book_services.h"

#include <list>
#include <iostream>
#include "boost/lexical_cast.hpp"
#include "json/json.h"  // jsoncpp

////////////////////////////////////////////////////////////////////////////////

// In-memory test data.
// There should be some database in a real product.

class Book {
public:
  std::string id;
  std::string title;
  double price;

  bool IsNull() const {
    return id.empty();
  }

  Json::Value ToJson() const {
    Json::Value root;
    root["id"] = id;
    root["title"] = title;
    root["price"] = price;
    return root;
  }
};

std::ostream& operator<<(std::ostream& os, const Book& book) {
  os << "{ " << book.id << ", " << book.title << ", " << book.price << " }";
  return os;
}

static const Book kNullBook{};

class BookStore {
public:
  BookStore() = default;

  const std::list<Book>& books() const {
    return books_;
  }

  const Book& GetBook(const std::string& id) const {
    auto it = FindBook(id);
    return (it == books_.end() ? kNullBook : *it);
  }

  bool AddBook(const Book& new_book) {
    if (FindBook(new_book.id) == books_.end()) {
      books_.push_back(new_book);
      return true;
    }
    return false;
  }

  bool UpdateBook(const Book& book) {
    auto it = FindBook(book.id);
    if (it != books_.end()) {
      it->title = book.title;
      it->price = book.price;
      return true;
    }
    return false;
  }

  bool DeleteBook(const std::string& id) {
    auto it = FindBook(id);

    if (it != books_.end()) {
      books_.erase(it);
      return true;
    }

    return false;
  }

private:
  std::list<Book>::const_iterator FindBook(const std::string& id) const {
    return std::find_if(books_.begin(),
                        books_.end(),
                        [&id](const Book& book) { return book.id == id; });
  }

  std::list<Book>::iterator FindBook(const std::string& id) {
    return std::find_if(books_.begin(),
                        books_.end(),
                        [&id](Book& book) { return book.id == id; });
  }

private:
  std::list<Book> books_;
};

static BookStore g_book_store;

////////////////////////////////////////////////////////////////////////////////

static bool BookFromJson(const std::string& json, Book* book) {
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::stringstream stream(json);
  std::string errs;
  if (!Json::parseFromStream(builder, stream, &root, &errs)) {
    std::cerr << errs << std::endl;
    return false;
  }

  book->id = root["id"].asString();
  book->title = root["title"].asString();
  book->price = root["price"].asDouble();

  return true;
}

// Return all books as a JSON array.
// TODO: Support query parameters.
bool BookListService::Get(const webcc::UrlQuery& /* query */,
                          std::string* response_content) {
  Json::Value root(Json::arrayValue);
  for (const Book& book : g_book_store.books()) {
    root.append(book.ToJson());
  }

  Json::StreamWriterBuilder builder;
  *response_content = Json::writeString(builder, root);

  return true;
}

// Add a new book.
// No response content.
bool BookListService::Post(const std::string& request_content,
                           std::string* /* response_content */) {
  Book book;
  if (BookFromJson(request_content, &book)) {
    return g_book_store.AddBook(book);
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool BookDetailService::Get(const std::vector<std::string>& url_sub_matches,
                            const webcc::UrlQuery& query,
                            std::string* response_content) {
  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  const Book& book = g_book_store.GetBook(book_id);
  if (!book.IsNull()) {
    Json::StreamWriterBuilder builder;
    *response_content = Json::writeString(builder, book.ToJson());
    return true;
  }

  return false;
}

// Update a book.
bool BookDetailService::Put(const std::vector<std::string>& url_sub_matches,
                            const std::string& request_content,
                            std::string* response_content) {
  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  Book book;
  if (BookFromJson(request_content, &book)) {
    book.id = book_id;
    return g_book_store.UpdateBook(book);
  }

  return false;
}

bool BookDetailService::Delete(const std::vector<std::string>& url_sub_matches) {
  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  return g_book_store.DeleteBook(book_id);
}
