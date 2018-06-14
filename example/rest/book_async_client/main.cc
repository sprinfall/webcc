#include <iostream>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/async_rest_client.h"

// -----------------------------------------------------------------------------

// Write a JSON object to string.
std::string JsonToString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, json);
}

// -----------------------------------------------------------------------------

class BookListClient {
 public:
  BookListClient(boost::asio::io_context& io_context,
                 const std::string& host, const std::string& port)
      : rest_client_(io_context, host, port) {
  }

  void ListBooks(webcc::HttpResponseHandler handler) {
    std::cout << "ListBooks" << std::endl;

    rest_client_.Get("/books", handler);
  }

  void CreateBook(const std::string& id,
                  const std::string& title,
                  double price,
                  webcc::HttpResponseHandler handler) {
    std::cout << "CreateBook: " << id << " " << title << " " << price
              << std::endl;

    Json::Value json(Json::objectValue);
    json["id"] = id;
    json["title"] = title;
    json["price"] = price;

    rest_client_.Post("/books", JsonToString(json), handler);
  }

 private:
  webcc::AsyncRestClient rest_client_;
};

// -----------------------------------------------------------------------------

class BookDetailClient {
public:
  BookDetailClient(boost::asio::io_context& io_context,
                   const std::string& host, const std::string& port)
      : rest_client_(io_context, host, port) {
  }

  void GetBook(const std::string& id, webcc::HttpResponseHandler handler) {
    std::cout << "GetBook: " << id << std::endl;

    rest_client_.Get("/book/" + id, handler);
  }

  void UpdateBook(const std::string& id,
                  const std::string& title,
                  double price,
                  webcc::HttpResponseHandler handler) {
    std::cout << "UpdateBook: " << id << " " << title << " " << price
              << std::endl;

    // NOTE: ID is already in the URL.
    Json::Value json(Json::objectValue);
    json["title"] = title;
    json["price"] = price;

    rest_client_.Put("/book/" + id, JsonToString(json), handler);
  }

  void DeleteBook(const std::string& id, webcc::HttpResponseHandler handler) {
    std::cout << "DeleteBook: " << id << std::endl;

    rest_client_.Delete("/book/" + id, handler);
  }

private:
  webcc::AsyncRestClient rest_client_;
};

// -----------------------------------------------------------------------------

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

  LOG_INIT(0);

  std::string host = argv[1];
  std::string port = argv[2];

  boost::asio::io_context io_context;

  BookListClient list_client(io_context, host, port);
  BookDetailClient detail_client(io_context, host, port);

  // Response handler.
  auto handler = [](std::shared_ptr<webcc::HttpResponse> response,
                    webcc::Error error,
                    bool timed_out) {
    if (error == webcc::kNoError) {
      std::cout << response->content() << std::endl;
    } else {
      std::cout << webcc::DescribeError(error) << std::endl;
    }
  };

  list_client.ListBooks(handler);
  list_client.CreateBook("1", "1984", 12.3, handler);

  detail_client.GetBook("1", handler);
  detail_client.UpdateBook("1", "1Q84", 32.1, handler);
  detail_client.GetBook("1", handler);
  detail_client.DeleteBook("1", handler);

  list_client.ListBooks(handler);

  io_context.run();

  return 0;
}
