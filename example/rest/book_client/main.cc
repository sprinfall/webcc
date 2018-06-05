#include <iostream>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/rest_client.h"

// -----------------------------------------------------------------------------

// Write a JSON object to string.
std::string JsonToString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, json);
}

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
  void PrintSeparateLine() {
    std::cout << "--------------------------------";
    std::cout << "--------------------------------";
    std::cout << std::endl;
  }

  void PrintError() {
    std::cout << webcc::DescribeError(rest_client_.error());
    if (rest_client_.timeout_occurred()) {
      std::cout << " (timeout)";
    }
    std::cout << std::endl;
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

  bool ListBooks() {
    PrintSeparateLine();
    std::cout << "ListBooks" << std::endl;

    if (!rest_client_.Get("/books")) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_content() << std::endl;
    return true;
  }

  bool CreateBook(const std::string& id,
                  const std::string& title,
                  double price) {
    PrintSeparateLine();
    std::cout << "CreateBook: " << id << ", " << title << ", " << price
              << std::endl;

    Json::Value json(Json::objectValue);
    json["id"] = id;
    json["title"] = title;
    json["price"] = price;

    if (!rest_client_.Post("/books", JsonToString(json))) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_status() << std::endl;

    return true;
  }
};

// -----------------------------------------------------------------------------

class BookDetailClient : public BookClientBase {
public:
  BookDetailClient(const std::string& host, const std::string& port,
                   int timeout_seconds)
      : BookClientBase(host, port, timeout_seconds) {
  }

  bool GetBook(const std::string& id) {
    PrintSeparateLine();
    std::cout << "GetBook: " << id << std::endl;

    if (!rest_client_.Get("/book/" + id)) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_content() << std::endl;
    return true;
  }

  bool UpdateBook(const std::string& id,
                  const std::string& title,
                  double price) {
    PrintSeparateLine();
    std::cout << "UpdateBook: " << id << ", " << title << ", " << price
              << std::endl;

    // NOTE: ID is already in the URL.
    Json::Value json(Json::objectValue);
    json["title"] = title;
    json["price"] = price;

    if (!rest_client_.Put("/book/" + id, JsonToString(json))) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_status() << std::endl;
    return true;
  }

  bool DeleteBook(const std::string& id) {
    PrintSeparateLine();
    std::cout << "DeleteBook: " << id << std::endl;

    if (!rest_client_.Delete("/book/" + id)) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_status() << std::endl;
    return true;
  }
};

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

  LOG_INIT(webcc::VERB, 0);

  std::string host = argv[1];
  std::string port = argv[2];

  int timeout_seconds = -1;
  if (argc > 3) {
    timeout_seconds = std::atoi(argv[3]);
  }

  BookListClient list_client(host, port, timeout_seconds);
  BookDetailClient detail_client(host, port, timeout_seconds);

  list_client.ListBooks();
  list_client.CreateBook("1", "1984", 12.3);

  detail_client.GetBook("1");
  detail_client.UpdateBook("1", "1Q84", 32.1);
  detail_client.GetBook("1");
  detail_client.DeleteBook("1");

  list_client.ListBooks();

  return 0;
}
