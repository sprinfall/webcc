#ifndef BOOK_SERVICES_H_
#define BOOK_SERVICES_H_

#include "webcc/rest_service.h"

////////////////////////////////////////////////////////////////////////////////

// BookListService handles the HTTP GET and returns the book list based on
// query parameters specified in the URL.
// The URL should be like:
//   - /books
//   - /books?name={BookName}
// The query parameters could be regular expressions.
class BookListService : public webcc::RestListService {
 protected:
  // Return a list of books based on query parameters.
  // URL examples:
  //   - /books
  //   - /books?name={BookName}
  bool Get(const webcc::UrlQuery& query,
           std::string* response_content) final;

  // Create a new book.
  bool Post(const std::string& request_content,
            std::string* response_content) final;
};

////////////////////////////////////////////////////////////////////////////////

// The URL is like '/books/{BookID}', and the 'url_sub_matches' parameter
// contains the matched book ID.
class BookDetailService : public webcc::RestDetailService {
 protected:
  bool Get(const std::vector<std::string>& url_sub_matches,
           std::string* response_content) final;

  bool Patch(const std::vector<std::string>& url_sub_matches,
             const std::string& request_content,
             std::string* response_content) final;

  bool Delete(const std::vector<std::string>& url_sub_matches) final;
};

#endif  // BOOK_SERVICE_H_
