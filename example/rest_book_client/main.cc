#include <iostream>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/rest_client.h"

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

static std::string JsonToString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, json);
}

static Json::Value StringToJson(const std::string& str) {
  Json::Value json;

  Json::CharReaderBuilder builder;
  std::stringstream stream(str);
  std::string errs;
  if (!Json::parseFromStream(builder, stream, &json, &errs)) {
    std::cerr << errs << std::endl;
  }

  return json;
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
    if (rest_client_.timed_out()) {
      std::cout << " (timed out)";
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

  bool CreateBook(const std::string& title, double price, std::string* id) {
    PrintSeparateLine();
    std::cout << "CreateBook: " << title << ", " << price << std::endl;

    Json::Value req_json(Json::objectValue);
    req_json["title"] = title;
    req_json["price"] = price;

    if (!rest_client_.Post("/books", JsonToString(req_json))) {
      PrintError();
      return false;
    }

    std::cout << rest_client_.response_status() << std::endl;

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

  bool UpdateBook(const std::string& id, const std::string& title,
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

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE_FILE_OVERWRITE);

  std::string host = argv[1];
  std::string port = argv[2];

  int timeout_seconds = -1;
  if (argc > 3) {
    timeout_seconds = std::atoi(argv[3]);
  }

  BookListClient list_client(host, port, timeout_seconds);
  BookDetailClient detail_client(host, port, timeout_seconds);

  list_client.ListBooks();

  std::string id;
  list_client.CreateBook("1984", 12.3, &id);

  detail_client.GetBook(id);
  detail_client.UpdateBook(id, "1Q84", 32.1);
  detail_client.GetBook(id);
  detail_client.DeleteBook(id);

  list_client.ListBooks();

  getchar();

  return 0;
}
