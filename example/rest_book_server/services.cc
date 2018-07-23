#include "example/rest_book_server/services.h"

#include <iostream>
#include <list>
#include <thread>

#include "json/json.h"
#include "webcc/logger.h"

#include "example/common/book.h"

// -----------------------------------------------------------------------------

static BookStore g_book_store;

static Json::Value BookToJson(const Book& book) {
  Json::Value root;
  root["id"] = book.id;
  root["title"] = book.title;
  root["price"] = book.price;
  return root;
}

static bool JsonToBook(const std::string& json, Book* book) {
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

// -----------------------------------------------------------------------------

// Return all books as a JSON array.
// TODO: Support query parameters.
bool BookListService::Get(const webcc::UrlQuery& /*query*/,
                          std::string* response_content) {
  if (sleep_seconds_ > 0) {
    LOG_INFO("Sleep %d seconds...", sleep_seconds_);
    std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
  }

  Json::Value root(Json::arrayValue);
  for (const Book& book : g_book_store.books()) {
    root.append(BookToJson(book));
  }

  Json::StreamWriterBuilder builder;
  *response_content = Json::writeString(builder, root);

  return true;
}

// Add a new book.
bool BookListService::Post(const std::string& request_content,
                           std::string* response_content) {
  if (sleep_seconds_ > 0) {
    LOG_INFO("Sleep %d seconds...", sleep_seconds_);
    std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
  }

  Book book;
  if (JsonToBook(request_content, &book)) {
    std::string id = g_book_store.AddBook(book);

    Json::Value root;
    root["id"] = id;

    Json::StreamWriterBuilder builder;
    *response_content = Json::writeString(builder, root);

    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------

bool BookDetailService::Get(const std::vector<std::string>& url_sub_matches,
                            const webcc::UrlQuery& query,
                            std::string* response_content) {
  if (sleep_seconds_ > 0) {
    LOG_INFO("Sleep %d seconds...", sleep_seconds_);
    std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
  }

  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  const Book& book = g_book_store.GetBook(book_id);
  if (!book.IsNull()) {
    Json::StreamWriterBuilder builder;
    *response_content = Json::writeString(builder, BookToJson(book));
    return true;
  }

  return false;
}

// Update a book.
bool BookDetailService::Put(const std::vector<std::string>& url_sub_matches,
                            const std::string& request_content,
                            std::string* response_content) {
  if (sleep_seconds_ > 0) {
    LOG_INFO("Sleep %d seconds...", sleep_seconds_);
    std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
  }

  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  Book book;
  if (JsonToBook(request_content, &book)) {
    book.id = book_id;
    return g_book_store.UpdateBook(book);
  }

  return false;
}

bool BookDetailService::Delete(
    const std::vector<std::string>& url_sub_matches) {
  if (sleep_seconds_ > 0) {
    LOG_INFO("Sleep %d seconds...", sleep_seconds_);
    std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
  }

  if (url_sub_matches.size() != 1) {
    return false;
  }

  const std::string& book_id = url_sub_matches[0];

  return g_book_store.DeleteBook(book_id);
}
