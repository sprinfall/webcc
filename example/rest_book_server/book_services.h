#ifndef BOOK_SERVICES_H_
#define BOOK_SERVICES_H_

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
  BookListService(int sleep_seconds) : sleep_seconds_(sleep_seconds) {
  }

 protected:
  // Return a list of books based on query parameters.
  // URL examples:
  //   - /books
  //   - /books?name={BookName}
  bool Get(const webcc::UrlQuery& query,
           std::string* response_content) override;

  // Create a new book.
  bool Post(const std::string& request_content,
            std::string* response_content) override;

 private:
   // Sleep for the client to test timeout control.
   int sleep_seconds_ = 0;
};

// -----------------------------------------------------------------------------

// The URL is like '/books/{BookID}', and the 'url_sub_matches' parameter
// contains the matched book ID.
class BookDetailService : public webcc::RestDetailService {
 public:
  BookDetailService(int sleep_seconds) : sleep_seconds_(sleep_seconds) {
  }

 protected:
  bool Get(const std::vector<std::string>& url_sub_matches,
           const webcc::UrlQuery& query,
           std::string* response_content) override;

  bool Put(const std::vector<std::string>& url_sub_matches,
           const std::string& request_content,
           std::string* response_content) override;

  bool Delete(const std::vector<std::string>& url_sub_matches) override;

 private:
  // Sleep for the client to test timeout control.
  int sleep_seconds_ = 0;
};

#endif  // BOOK_SERVICE_H_
