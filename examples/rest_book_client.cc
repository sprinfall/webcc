#include <iostream>
#include <list>

#include "json/json.h"

#include "webcc/client_session.h"
#include "webcc/logger.h"

#include "examples/common/book.h"
#include "examples/common/book_json.h"

#if (defined(_WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

// -----------------------------------------------------------------------------

class BookClientBase {
public:
  BookClientBase(webcc::ClientSession& session, const std::string& url)
      : session_(session), url_(url) {
  }

  virtual ~BookClientBase() = default;

protected:
  // Check HTTP response status.
  bool CheckStatus(webcc::ResponsePtr response, int expected_status) {
    int status = response->status();
    if (status != expected_status) {
      LOG_ERRO("HTTP status error (actual: %d, expected: %d).",
               status, expected_status);
      return false;
    }
    return true;
  }

protected:
  std::string url_;

  webcc::ClientSession& session_;
};

// -----------------------------------------------------------------------------

class BookListClient : public BookClientBase {
public:
  BookListClient(webcc::ClientSession& session, const std::string& url)
      : BookClientBase(session, url) {
  }

  bool ListBooks(std::list<Book>* books) {
    try {
      auto r = session_.Get(url_ + "/books");
      
      if (!CheckStatus(r, webcc::Status::kOK)) {
        // Response HTTP status error.
        return false;
      }

      Json::Value rsp_json = StringToJson(r->data());

      if (!rsp_json.isArray()) {
        return false;  // Should be a JSON array of books.
      }

      for (Json::ArrayIndex i = 0; i < rsp_json.size(); ++i) {
        books->push_back(JsonToBook(rsp_json[i]));
      }

      return true;

    } catch (const webcc::Error& error) {
      std::cerr << error << std::endl;
      return false;
    }
  }

  bool CreateBook(const std::string& title, double price, std::string* id) {
    Json::Value req_json;
    req_json["title"] = title;
    req_json["price"] = price;

    try {
      auto r = session_.Post(url_ + "/books", JsonToString(req_json), true);

      if (!CheckStatus(r, webcc::Status::kCreated)) {
        return false;
      }

      Json::Value rsp_json = StringToJson(r->data());
      *id = rsp_json["id"].asString();

      return !id->empty();

    } catch (const webcc::Error& error) {
      std::cerr << error << std::endl;
      return false;
    }
  }
};

// -----------------------------------------------------------------------------

class BookDetailClient : public BookClientBase {
public:
  BookDetailClient(webcc::ClientSession& session, const std::string& url)
      : BookClientBase(session, url) {
  }

  bool GetBook(const std::string& id, Book* book) {
    try {
      auto r = session_.Get(url_ + "/books/" + id);

      if (!CheckStatus(r, webcc::Status::kOK)) {
        return false;
      }

      return JsonStringToBook(r->data(), book);

    } catch (const webcc::Error& error) {
      std::cerr << error << std::endl;
      return false;
    }
  }

  bool UpdateBook(const std::string& id, const std::string& title,
                  double price) {
    Json::Value json;
    json["title"] = title;
    json["price"] = price;

    try {
      auto r = session_.Put(url_ + "/books/" + id, JsonToString(json), true);

      if (!CheckStatus(r, webcc::Status::kOK)) {
        return false;
      }

      return true;

    } catch (const webcc::Error& error) {
      std::cerr << error << std::endl;
      return false;
    }
  }

  bool DeleteBook(const std::string& id) {
    try {
      auto r = session_.Delete(url_ + "/books/" + id);

      if (!CheckStatus(r, webcc::Status::kOK)) {
        return false;
      }

      return true;

    } catch (const webcc::Error& error) {
      std::cerr << error << std::endl;
      return false;
    }
  }
};

// -----------------------------------------------------------------------------

void PrintSeparator() {
  static const std::string s_line(80, '-');
  std::cout << s_line << std::endl;
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
  std::cout << "Usage: " << argv0 << " <url> [timeout]" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << "http://localhost:8080" << std::endl;
  std::cout << "    " << argv0 << "http://localhost:8080 2" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  std::string url = argv[1];

  int timeout = 0;
  if (argc > 2) {
    timeout = std::atoi(argv[2]);
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE_FILE_OVERWRITE);

  // Share the same session.
  webcc::ClientSession session;

  session.set_timeout(timeout);

  // If the request has body, default to this content type.
  // Optional.
  session.set_media_type("application/json");
  session.set_charset("utf-8");

  BookListClient list_client(session, url);
  BookDetailClient detail_client(session, url);

  PrintSeparator();

  std::list<Book> books;
  if (list_client.ListBooks(&books)) {
    PrintBookList(books);
  }

  PrintSeparator();

  std::string id;
  if (list_client.CreateBook("1984", 12.3, &id)) {
    std::cout << "Book ID: " << id << std::endl;
  } else {
    id = "1";
    std::cout << "Book ID: " << id << " (faked)"<< std::endl;
  }

  PrintSeparator();

  books.clear();
  if (list_client.ListBooks(&books)) {
    PrintBookList(books);
  }

  PrintSeparator();

  Book book;
  if (detail_client.GetBook(id, &book)) {
    PrintBook(book);
  }

  PrintSeparator();

  detail_client.UpdateBook(id, "1Q84", 32.1);

  PrintSeparator();

  if (detail_client.GetBook(id, &book)) {
    PrintBook(book);
  }

  PrintSeparator();

  detail_client.DeleteBook(id);

  PrintSeparator();

  books.clear();
  if (list_client.ListBooks(&books)) {
    PrintBookList(books);
  }

  return 0;
}
