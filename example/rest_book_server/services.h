#ifndef EXAMPLE_REST_BOOK_SERVER_SERVICES_H_
#define EXAMPLE_REST_BOOK_SERVER_SERVICES_H_

#include <string>
#include <vector>

#include "webcc/rest_service.h"

// -----------------------------------------------------------------------------

class BookListService : public webcc::RestListService {
public:
  explicit BookListService(int sleep_seconds)
      : sleep_seconds_(sleep_seconds) {
  }

public:
  // Get a list of books based on query parameters.
  void Get(const webcc::UrlQuery& query, webcc::RestResponse* response) final;

  // Create a new book.
  void Post(const std::string& request_content,
            webcc::RestResponse* response) final;

private:
  // Sleep some seconds before send back the response.
  // For testing timeout control in client side.
  int sleep_seconds_;
};

// -----------------------------------------------------------------------------

// The URL is like '/books/{BookID}', and the 'url_sub_matches' parameter
// contains the matched book ID.
class BookDetailService : public webcc::RestDetailService {
public:
  explicit BookDetailService(int sleep_seconds)
      : sleep_seconds_(sleep_seconds) {
  }

public:
  // Get the detailed information of a book.
  void Get(const webcc::UrlSubMatches& url_sub_matches,
           const webcc::UrlQuery& query,
           webcc::RestResponse* response) final;

  // Update a book.
  void Put(const webcc::UrlSubMatches& url_sub_matches,
           const std::string& request_content,
           webcc::RestResponse* response) final;

  // Delete a book.
  void Delete(const webcc::UrlSubMatches& url_sub_matches,
              webcc::RestResponse* response) final;

private:
  // Sleep some seconds before send back the response.
  // For testing timeout control in client side.
  int sleep_seconds_;
};

#endif  // EXAMPLE_REST_BOOK_SERVER_SERVICES_H_
