#include <iostream>

#include "boost/algorithm/string.hpp"
#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/rest_client.h"

////////////////////////////////////////////////////////////////////////////////

// Write a JSON object to string.
std::string JsonToString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, json);
}

////////////////////////////////////////////////////////////////////////////////

class BookListClient {
public:
  BookListClient(const std::string& host, const std::string& port)
      : rest_client_(host, port) {
  }

  bool ListBooks() {
    std::cout << "ListBooks" << std::endl;

    webcc::HttpResponse http_response;
    if (!rest_client_.Get("/books", &http_response)) {
      return false;
    }

    std::cout << http_response.content() << std::endl;
    return true;
  }

  bool CreateBook(const std::string& id,
                  const std::string& title,
                  double price) {
    std::cout << "CreateBook: " << id << " " << title << " " << price
              << std::endl;

    Json::Value json(Json::objectValue);
    json["id"] = id;
    json["title"] = title;
    json["price"] = price;

    webcc::HttpResponse http_response;
    if (!rest_client_.Post("/books", JsonToString(json), &http_response)) {
      return false;
    }

    std::cout << http_response.status() << std::endl;

    return true;
  }

private:
  webcc::RestClient rest_client_;
};

////////////////////////////////////////////////////////////////////////////////

class BookDetailClient {
public:
  BookDetailClient(const std::string& host, const std::string& port)
      : rest_client_(host, port) {
  }

  bool GetBook(const std::string& id) {
    std::cout << "GetBook: " << id << std::endl;

    webcc::HttpResponse http_response;
    if (!rest_client_.Get("/book/" + id, &http_response)) {
      return false;
    }

    std::cout << http_response.content() << std::endl;
    return true;
  }

  bool UpdateBook(const std::string& id,
                  const std::string& title,
                  double price) {
    std::cout << "UpdateBook: " << id << " " << title << " " << price
              << std::endl;

    // NOTE: ID is already in the URL.
    Json::Value json(Json::objectValue);
    json["title"] = title;
    json["price"] = price;

    webcc::HttpResponse http_response;
    if (!rest_client_.Put("/book/" + id, JsonToString(json), &http_response)) {
      return false;
    }

    std::cout << http_response.status() << std::endl;
    return true;
  }

  bool DeleteBook(const std::string& id) {
    std::cout << "DeleteBook: " << id << std::endl;

    webcc::HttpResponse http_response;
    if (!rest_client_.Delete("/book/" + id, &http_response)) {
      return false;
    }

    std::cout << http_response.status() << std::endl;
    return true;
  }

private:
  webcc::RestClient rest_client_;
};

////////////////////////////////////////////////////////////////////////////////

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " localhost 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    Help(argv[0]);
    return 1;
  }

  LOG_INIT(webcc::ERRO, 0);

  std::string host = argv[1];
  std::string port = argv[2];

  BookListClient list_client(host, port);
  BookDetailClient detail_client(host, port);

  list_client.ListBooks();
  list_client.CreateBook("1", "1984", 12.3);

  detail_client.GetBook("1");
  detail_client.UpdateBook("1", "1Q84", 32.1);
  detail_client.GetBook("1");
  detail_client.DeleteBook("1");

  list_client.ListBooks();

  return 0;
}
