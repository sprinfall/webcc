#include <iostream>

#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

class BookListClient {
public:
  BookListClient() {
    host_ = "localhost";
    port_ = "8080";
  }

  bool ListBooks() {
    webcc::HttpRequest http_request;

    http_request.set_method(webcc::kHttpGet);
    http_request.set_url("/books");
    http_request.SetHost(host_, port_);

    http_request.Build();

    webcc::HttpResponse http_response;

    webcc::HttpClient http_client;
    webcc::Error error = http_client.SendRequest(http_request, &http_response);

    if (error != webcc::kNoError) {
      return false;
    }

    std::cout << "Book list: " << std::endl
              << http_response.content() << std::endl;

    return true;
  }

private:
  std::string host_;
  std::string port_;
};

class BookDetailClient {
public:
  BookDetailClient() {
    host_ = "localhost";
    port_ = "8080";
  }

  bool GetBook(const std::string& id) {
    webcc::HttpRequest http_request;

    http_request.set_method(webcc::kHttpGet);
    http_request.set_url("/books/" + id);
    http_request.SetHost(host_, port_);

    http_request.Build();

    webcc::HttpResponse http_response;

    webcc::HttpClient http_client;
    webcc::Error error = http_client.SendRequest(http_request, &http_response);

    if (error != webcc::kNoError) {
      return false;
    }

    std::cout << "Book: " << id << std::endl
              << http_response.content() << std::endl;

    return true;
  }

private:
  std::string host_;
  std::string port_;
};

int main() {
  BookListClient book_list_client;
  book_list_client.ListBooks();

  BookDetailClient book_detail_client;
  book_detail_client.GetBook("1");

  return 0;
}
