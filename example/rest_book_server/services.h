#ifndef EXAMPLE_REST_BOOK_SERVER_SERVICES_H_
#define EXAMPLE_REST_BOOK_SERVER_SERVICES_H_

#include <string>
#include <vector>

#include "webcc/rest_service.h"

// -----------------------------------------------------------------------------

// BookListService handles the HTTP GET and returns the book list based on
// query parameters specified in the URL.
// The URL should be like:
//   - /books
//   - /books?name={BookName}
// The query parameters could be regular expressions.
class BookListService : public webcc::RestListService {
 public:
  explicit BookListService(int sleep_seconds)
      : sleep_seconds_(sleep_seconds) {
  }

 protected:
  // Get a list of books based on query parameters.
  // URL examples:
  //   - /books
  //   - /books?name={BookName}
  void Get(const webcc::UrlQuery& query,
           webcc::RestResponse* response) final;

  // Create a new book.
  void Post(const std::string& request_content,
            webcc::RestResponse* response) final;

 private:
   // Sleep for the client to test timeout control.
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

 protected:
  void Get(const std::vector<std::string>& url_sub_matches,
           const webcc::UrlQuery& query,
           webcc::RestResponse* response) final;

  void Put(const std::vector<std::string>& url_sub_matches,
           const std::string& request_content,
           webcc::RestResponse* response) final;

  void Delete(const std::vector<std::string>& url_sub_matches,
              webcc::RestResponse* response) final;

 private:
  // Sleep for the client to test timeout control.
  int sleep_seconds_;
};

#endif  // EXAMPLE_REST_BOOK_SERVER_SERVICES_H_
