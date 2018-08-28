#include <iostream>
#include <list>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/rest_client.h"

#include "example/common/book.h"
#include "example/common/book_json.h"

// In order to run with VLD, please copy the following files to the example
// output folder from "third_party\win32\bin":
//   - dbghelp.dll
//   - Microsoft.DTfW.DHL.manifest
//   - vld_x86.dll
#if (defined(WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

// -----------------------------------------------------------------------------

class BookClientBase {
 public:
  BookClientBase(const std::string& host, const std::string& port,
                 int timeout_seconds)
      : rest_client_(host, port) {
    rest_client_.set_timeout_seconds(timeout_seconds);
  }

  virtual ~BookClientBase() = default;

 protected:
  // Log the socket communication error.
  void LogError() {
    if (rest_client_.timed_out()) {
      LOG_ERRO("%s (timed out)", webcc::DescribeError(rest_client_.error()));
    } else {
      LOG_ERRO(webcc::DescribeError(rest_client_.error()));
    }
  }

  // Check HTTP response status.
  bool CheckStatus(webcc::HttpStatus::Enum expected_status) {
    int status = rest_client_.response_status();
    if (status != expected_status) {
      LOG_ERRO("HTTP status error (actual: %d, expected: %d).",
               status, expected_status);
      return false;
    }
    return true;
  }

  webcc::RestClient rest_client_;
};

// -----------------------------------------------------------------------------

class BookListClient : public BookClientBase {
 public:
  BookListClient(const std::string& host, const std::string& port,
                 int timeout_seconds)
      : BookClientBase(host, port, timeout_seconds) {
  }

  bool ListBooks(std::list<Book>* books) {
    if (!rest_client_.Get("/books")) {
      // Socket communication error.
      LogError();
      return false;
    }

    if (!CheckStatus(webcc::HttpStatus::kOK)) {
      // Response HTTP status error.
      return false;
    }

    Json::Value rsp_json = StringToJson(rest_client_.response_content());

    if (!rsp_json.isArray()) {
      return false;  // Should be a JSON array of books.
    }

    for (Json::ArrayIndex i = 0; i < rsp_json.size(); ++i) {
      books->push_back(JsonToBook(rsp_json[i]));
    }

    return true;
  }

  bool CreateBook(const std::string& title, double price, std::string* id) {
    Json::Value req_json;
    req_json["title"] = title;
    req_json["price"] = price;

    if (!rest_client_.Post("/books", JsonToString(req_json))) {
      LogError();
      return false;
    }

    if (!CheckStatus(webcc::HttpStatus::kCreated)) {
      return false;
    }

    Json::Value rsp_json = StringToJson(rest_client_.response_content());
    *id = rsp_json["id"].asString();

    return !id->empty();
  }
};

// -----------------------------------------------------------------------------

class BookDetailClient : public BookClientBase {
 public:
  BookDetailClient(const std::string& host, const std::string& port,
                   int timeout_seconds)
      : BookClientBase(host, port, timeout_seconds) {
  }

  bool GetBook(const std::string& id, Book* book) {
    if (!rest_client_.Get("/books/" + id)) {
      LogError();
      return false;
    }

    if (!CheckStatus(webcc::HttpStatus::kOK)) {
      return false;
    }

    return JsonStringToBook(rest_client_.response_content(), book);
  }

  bool UpdateBook(const std::string& id, const std::string& title,
                  double price) {
    Json::Value json;
    json["title"] = title;
    json["price"] = price;

    if (!rest_client_.Put("/books/" + id, JsonToString(json))) {
      LogError();
      return false;
    }

    if (!CheckStatus(webcc::HttpStatus::kOK)) {
      return false;
    }

    return true;
  }

  bool DeleteBook(const std::string& id) {
    if (!rest_client_.Delete("/books/" + id)) {
      LogError();
      return false;
    }

    if (!CheckStatus(webcc::HttpStatus::kOK)) {
      return false;
    }

    return true;
  }
};

// -----------------------------------------------------------------------------

void PrintSeparator() {
  std::cout << std::string(80, '-') << std::endl;
}

void PrintBook(const Book& book) {
  std::cout << "Book: " << book << std::endl;
}

void PrintBookList(const std::list<Book>& books) {
  std::cout << "Book list: " << books.size() << std::endl;
  for (const Book& book : books) {
    std::cout << "  Book: " << book << std::endl;
  }
}

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <port> [timeout]" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " localhost 8080" << std::endl;
  std::cout << "    " << argv0 << " localhost 8080 2" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE_FILE_OVERWRITE);

  std::string host = argv[1];
  std::string port = argv[2];

  int timeout_seconds = -1;
  if (argc > 3) {
    timeout_seconds = std::atoi(argv[3]);
  }

  BookListClient list_client(host, port, timeout_seconds);
  BookDetailClient detail_client(host, port, timeout_seconds);

  PrintSeparator();

  std::list<Book> books;
  list_client.ListBooks(&books);
  PrintBookList(books);

  PrintSeparator();

  std::string id;
  if (!list_client.CreateBook("1984", 12.3, &id)) {
    return 1;
  }
  std::cout << "Book ID: " << id << std::endl;

  PrintSeparator();

  books.clear();
  list_client.ListBooks(&books);
  PrintBookList(books);

  PrintSeparator();

  Book book;
  detail_client.GetBook(id, &book);
  PrintBook(book);

  PrintSeparator();

  detail_client.UpdateBook(id, "1Q84", 32.1);

  PrintSeparator();

  detail_client.GetBook(id, &book);
  PrintBook(book);

  PrintSeparator();

  detail_client.DeleteBook(id);

  PrintSeparator();

  books.clear();
  list_client.ListBooks(&books);
  PrintBookList(books);

  return 0;
}
