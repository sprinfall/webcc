#ifndef BOOK_SERVICES_H_
#define BOOK_SERVICES_H_

#include "webcc/rest_service.h"

// NOTE:
// XxxListService and XxxDetailService are similar to the XxxListView
// and XxxDetailView in Django (a Python web framework).

// List Service handles the HTTP GET and returns the book list based on
// query parameters specified in the URL.
// The URL should be like:
//   - /books
//   - /books?name={BookName} 
// The query parameters could be regular expressions.
class BookListService : public webcc::RestService {
public:
  BookListService() = default;
  ~BookListService() override = default;

  bool Handle(const std::string& http_method,
              const std::vector<std::string>& url_sub_matches,
              const std::string& request_content,
              std::string* response_content) override;
};

// Detail Service handles the following HTTP methods:
//   - GET
//   - PUT
//   - PATCH
//   - DELETE
// The URL should be like: /books/{BookID}.
class BookDetailService : public webcc::RestService {
public:
  BookDetailService() = default;
  ~BookDetailService() override = default;

  bool Handle(const std::string& http_method,
              const std::vector<std::string>& url_sub_matches,
              const std::string& request_content,
              std::string* response_content) override;
};

#endif  // BOOK_SERVICE_H_
