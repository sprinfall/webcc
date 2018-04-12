#include "book_services.h"

#include <list>
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

static const Book kNullBook{};

class BookStore {
public:
  BookStore() {
    // Prepare test data.
    books_.push_back({ "1", "Title1", 11.1 });
    books_.push_back({ "2", "Title2", 22.2 });
    books_.push_back({ "3", "Title3", 33.3 });
  }

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

private:
  std::list<Book> books_;
};

static BookStore g_book_store;

////////////////////////////////////////////////////////////////////////////////

bool BookListService::Handle(const std::string& http_method,
                             const std::vector<std::string>& url_sub_matches,
                             const std::string& request_content,
                             std::string* response_content) {
  if (http_method == webcc::kHttpGet) {
    Json::Value root(Json::arrayValue);
    for (const Book& book : g_book_store.books()) {
      root.append(book.ToJson());
    }

    Json::StreamWriterBuilder wbuilder;
    *response_content = Json::writeString(wbuilder, root);
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

    Json::StreamWriterBuilder wbuilder;
    *response_content = Json::writeString(wbuilder, book.ToJson());

    return true;

  } else if (http_method == webcc::kHttpPost) {

  } else if (http_method == webcc::kHttpDelete) {

  }

  return false;
}
