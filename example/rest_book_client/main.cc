#include <iostream>

#include "boost/algorithm/string.hpp"
#include "json/json.h"  // jsoncpp

#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

////////////////////////////////////////////////////////////////////////////////

class BookClientBase {
public:
  BookClientBase(const std::string& host, const std::string& port)
      : host_(host), port_(port) {
  }

  bool Request(const std::string& method,
               const std::string& url,
               const std::string& content,
               webcc::HttpResponse* http_response) {
    webcc::HttpRequest http_request;

    http_request.set_method(method);
    http_request.set_url(url);
    http_request.SetHost(host_, port_);
    if (!content.empty()) {  // TODO
      http_request.SetContent(content);
    }
    http_request.Build();

    webcc::HttpClient http_client;
    webcc::Error error = http_client.SendRequest(http_request, http_response);

    return error == webcc::kNoError;
  }

protected:
  std::string host_;
  std::string port_;
};

////////////////////////////////////////////////////////////////////////////////

class BookListClient : public BookClientBase {
public:
  BookListClient(const std::string& host, const std::string& port)
      : BookClientBase(host, port) {
  }

  bool ListBooks() {
    webcc::HttpResponse http_response;
    if (!Request(webcc::kHttpGet, "/books", "", &http_response)) {
      return false;
    }

    std::cout << http_response.content() << std::endl;

    return true;
  }

  bool CreateBook(const std::string& id,
                  const std::string& title,
                  double price) {
    Json::Value root(Json::objectValue);
    root["id"] = id;
    root["title"] = title;
    root["price"] = price;

    Json::StreamWriterBuilder builder;
    std::string book_json = Json::writeString(builder, root);

    webcc::HttpResponse http_response;
    if (!Request(webcc::kHttpPost, "/books", book_json, &http_response)) {
      return false;
    }

    std::cout << http_response.status() << std::endl;

    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////

class BookDetailClient : public BookClientBase {
public:
  BookDetailClient(const std::string& host, const std::string& port)
      : BookClientBase(host, port) {
  }

  bool GetBook(const std::string& id) {
    webcc::HttpResponse http_response;
    if (!Request(webcc::kHttpGet, "/books/" + id, "", &http_response)) {
      return false;
    }

    std::cout << http_response.content() << std::endl;

    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////

std::string g_host;
std::string g_port;

void ListBooks() {
  std::cout << "List books" << std::endl;

  BookListClient client(g_host, g_port);

  client.ListBooks();
}

void CreateBook(const std::string& id,
                const std::string& title,
                double price) {
  std::cout << "Create book: " <<  id << " " << title << " " << price << std::endl;

  BookListClient client(g_host, g_port);

  client.CreateBook(id, title, price);
}

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " localhost 8080" << std::endl;
}

std::string GetUserInput() {
  char input[256];
  std::size_t length = 0;
  do {
    std::cout << ">> ";
    std::cin.getline(input, 256);
    length = strlen(input);
  } while (length == 0);

  return input;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    Help(argv[0]);
    return 1;
  }

  g_host = argv[1];
  g_port = argv[2];

  // Type commands to execute actions.
  // Commands: list, create, update, remove
  // Examples:
  //   >> list
  //   >> create 1 { "title": "1984", "price": 12.3 }
  //   >> detail 1
  //   >> update 1 { "price": 32 }
  //   >> delete 1

  while (true) {
    std::string input = GetUserInput();
    boost::trim(input);

    std::string command;
    std::size_t i = input.find(' ');
    if (i == std::string::npos) {
      command = input;
    } else {
      command = input.substr(0, i);
    }

    if (command == "exit") {
      break;
    }

    if (command == "list") {
      ListBooks();
      continue;
    }

    ++i;

    std::size_t j = input.find(' ', i);
    std::string id = input.substr(i, j - i);
    i = j + 1;

    if (command == "create") {
      std::string json = input.substr(i);
      std::cout << "json:" << json << std::endl;

      Json::Value root;
      Json::CharReaderBuilder builder;
      std::string errs;
      if (Json::parseFromStream(builder, std::stringstream(json), &root, &errs)) {
        CreateBook(id, root["title"].asString(), root["price"].asDouble());
      } else {
        std::cerr << errs << std::endl;
      }
    }
  }

  //BookDetailClient book_detail_client(host, port);
  //book_detail_client.GetBook("1");

  return 0;
}
