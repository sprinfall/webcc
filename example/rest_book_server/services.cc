#include "example/rest_book_server/services.h"

#include <iostream>
#include <list>
#include <thread>

#include "json/json.h"
#include "webcc/logger.h"

#include "example/common/book.h"
#include "example/common/book_json.h"

// -----------------------------------------------------------------------------

static BookStore g_book_store;

static void Sleep(int seconds) {
  if (seconds > 0) {
    LOG_INFO("Sleep %d seconds...", seconds);
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }
}

// -----------------------------------------------------------------------------

// Return all books as a JSON array.
void BookListService::Get(const webcc::UrlQuery& /*query*/,
                          webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  Json::Value json(Json::arrayValue);

  for (const Book& book : g_book_store.books()) {
    json.append(BookToJson(book));
  }

  response->content = JsonToString(json);
  response->status = webcc::HttpStatus::kOK;
}

void BookListService::Post(const std::string& request_content,
                           webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  Book book;
  if (JsonStringToBook(request_content, &book)) {
    std::string id = g_book_store.AddBook(book);

    Json::Value json;
    json["id"] = id;

    response->content = JsonToString(json);
    response->status = webcc::HttpStatus::kCreated;
  } else {
    // Invalid JSON
    response->status = webcc::HttpStatus::kBadRequest;
  }
}

// -----------------------------------------------------------------------------

void BookDetailService::Get(const webcc::UrlSubMatches& url_sub_matches,
                            const webcc::UrlQuery& query,
                            webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_sub_matches.size() != 1) {
    // Using kNotFound means the resource specified by the URL cannot be found.
    // kBadRequest could be another choice.
    response->status = webcc::HttpStatus::kNotFound;
    return;
  }

  const std::string& book_id = url_sub_matches[0];

  const Book& book = g_book_store.GetBook(book_id);
  if (book.IsNull()) {
    response->status = webcc::HttpStatus::kNotFound;
    return;
  }

  response->content = BookToJsonString(book);
  response->status = webcc::HttpStatus::kOK;
}

void BookDetailService::Put(const webcc::UrlSubMatches& url_sub_matches,
                            const std::string& request_content,
                            webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_sub_matches.size() != 1) {
    response->status = webcc::HttpStatus::kNotFound;
    return;
  }

  const std::string& book_id = url_sub_matches[0];

  Book book;
  if (!JsonStringToBook(request_content, &book)) {
    response->status = webcc::HttpStatus::kBadRequest;
    return;
  }

  book.id = book_id;
  g_book_store.UpdateBook(book);

  response->status = webcc::HttpStatus::kOK;
}

void BookDetailService::Delete(const webcc::UrlSubMatches& url_sub_matches,
                               webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_sub_matches.size() != 1) {
    response->status = webcc::HttpStatus::kNotFound;
    return;
  }

  const std::string& book_id = url_sub_matches[0];

  if (!g_book_store.DeleteBook(book_id)) {
    response->status = webcc::HttpStatus::kNotFound;
    return;
  }

  response->status = webcc::HttpStatus::kOK;
}
