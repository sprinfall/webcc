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
    webcc::HttpResponse http_response;
    if (!rest_client_.Get("/books", &http_response)) {
      return false;
    }

    std::cout << "result:\n" << http_response.content() << std::endl;
    return true;
  }

  bool CreateBook(const std::string& id,
                  const std::string& title,
                  double price) {
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
    // NOTE: ID is already in the URL.
    Json::Value json(Json::objectValue);
    json["title"] = title;
    json["price"] = price;

    webcc::HttpResponse http_response;
    if (!rest_client_.Post("/book/" + id, JsonToString(json), &http_response)) {
      return false;
    }

    std::cout << http_response.status() << std::endl;
    return true;
  }

  bool DeleteBook(const std::string& id) {
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

std::string g_host;
std::string g_port;

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <host> <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " localhost 8080" << std::endl;
}

std::string GetUserInput() {
  char input[256];
  std::cout << ">> ";
  std::cin.getline(input, 256);
  return input;
}

bool ParseJsonInput(const std::string& input, Json::Value* root) {
  Json::CharReaderBuilder builder;
  std::stringstream stream(input);
  std::string errs;
  if (Json::parseFromStream(builder, stream, root, &errs)) {
    return true;
  } else {
    std::cerr << errs << std::endl;
    return false;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    Help(argv[0]);
    return 1;
  }

  LOG_INIT(webcc::VERB, 0);

  g_host = argv[1];
  g_port = argv[2];

  // Type commands to execute actions.
  // Commands: list, create, detail, update, delete and exit.
  // Examples:
  //   >> list
  //   >> create 1 { "title": "1984", "price": 12.3 }
  //   >> detail 1
  //   >> update 1 { "title": "1Q84", "price": 32.1 }
  //   >> delete 1
  //   >> exit

  // A very naive implementation of interaction mode.
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
      BookListClient client(g_host, g_port);
      client.ListBooks();
      continue;
    }

    ++i;

    std::size_t j = input.find(' ', i);
    std::string id = input.substr(i, j - i);
    i = j + 1;

    if (command == "create") {
      std::string json = input.substr(i);

      Json::Value root;
      if (ParseJsonInput(json, &root)) {
        BookListClient client(g_host, g_port);
        client.CreateBook(id, root["title"].asString(), root["price"].asDouble());
      }
      continue;
    }

    if (command == "update") {
      std::string json = input.substr(i);

      Json::Value root;
      if (ParseJsonInput(json, &root)) {
        BookDetailClient client(g_host, g_port);
        client.UpdateBook(id, root["title"].asString(), root["price"].asDouble());
      }
      continue;
    }

    if (command == "detail") {
      BookDetailClient client(g_host, g_port);
      client.GetBook(id);
      continue;
    }

    if (command == "delete") {
      BookDetailClient client(g_host, g_port);
      client.DeleteBook(id);
      continue;
    }
  }


  return 0;
}
