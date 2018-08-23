#include <iostream>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/rest_async_client.h"

// -----------------------------------------------------------------------------

// Write a JSON object to string.
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
  BookClientBase(boost::asio::io_context& io_context,
                 const std::string& host, const std::string& port,
                 int timeout_seconds)
      : rest_client_(io_context, host, port) {
    rest_client_.set_timeout_seconds(timeout_seconds);
  }

  virtual ~BookClientBase() = default;

 protected:
  void PrintSeparateLine() {
    std::cout << "--------------------------------";
    std::cout << "--------------------------------";
    std::cout << std::endl;
  }

  // Generic response handler for RestAsyncClient APIs.
  void GenericHandler(std::function<void(webcc::HttpResponsePtr)> rsp_callback,
                      webcc::HttpResponsePtr response,
                      webcc::Error error,
                      bool timed_out) {
    if (error != webcc::kNoError) {
      std::cout << webcc::DescribeError(error);
      if (timed_out) {
        std::cout << " (timed out)";
      }
      std::cout << std::endl;
    } else {
      // Call the response callback on success.
      rsp_callback(response);
    }
  }

  webcc::RestAsyncClient rest_client_;
};

// -----------------------------------------------------------------------------

class BookListClient : public BookClientBase {
 public:
  BookListClient(boost::asio::io_context& io_context,
                 const std::string& host, const std::string& port,
                 int timeout_seconds)
      : BookClientBase(io_context, host, port, timeout_seconds) {
  }

  void ListBooks(webcc::HttpResponseHandler handler) {
    std::cout << "ListBooks" << std::endl;

    rest_client_.Get("/books", handler);
  }

  void CreateBook(const std::string& title, double price,
                  std::function<void(std::string)> id_callback) {
    std::cout << "CreateBook: " << title << " " << price << std::endl;

    Json::Value json(Json::objectValue);
    json["title"] = title;
    json["price"] = price;

    auto rsp_callback = [id_callback](webcc::HttpResponsePtr response) {
      Json::Value rsp_json = StringToJson(response->content());
      id_callback(rsp_json["id"].asString());
    };

    rest_client_.Post("/books", JsonToString(json),
                      std::bind(&BookListClient::GenericHandler, this,
                                rsp_callback,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3));
  }
};

// -----------------------------------------------------------------------------

class BookDetailClient : public BookClientBase {
 public:
  BookDetailClient(boost::asio::io_context& io_context,
                   const std::string& host, const std::string& port,
                   int timeout_seconds)
      : BookClientBase(io_context, host, port, timeout_seconds) {
  }

  void GetBook(const std::string& id, webcc::HttpResponseHandler handler) {
    std::cout << "GetBook: " << id << std::endl;

    auto rsp_callback = [](webcc::HttpResponsePtr response) {
      Json::Value rsp_json = StringToJson(response->content());

      //id_callback(rsp_json["id"].asString());
    };

    rest_client_.Get("/books/" + id, handler);
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

    rest_client_.Put("/books/" + id, JsonToString(json), handler);
  }

  void DeleteBook(const std::string& id, webcc::HttpResponseHandler handler) {
    std::cout << "DeleteBook: " << id << std::endl;

    rest_client_.Delete("/books/" + id, handler);
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

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::string host = argv[1];
  std::string port = argv[2];

  int timeout_seconds = -1;
  if (argc > 3) {
    timeout_seconds = std::atoi(argv[3]);
  }

  boost::asio::io_context io_context;

  BookListClient list_client(io_context, host, port, timeout_seconds);
  BookDetailClient detail_client(io_context, host, port, timeout_seconds);

  // Response handler.
  auto handler = [](webcc::HttpResponsePtr response, webcc::Error error,
                    bool timed_out) {
    if (error == webcc::kNoError) {
      std::cout << response->content() << std::endl;
    } else {
      std::cout << webcc::DescribeError(error);
      if (timed_out) {
        std::cout << " (timed out)";
      }
      std::cout << std::endl;
    }
  };

  list_client.ListBooks(handler);

  list_client.CreateBook("1984", 12.3, [](std::string id) {
    std::cout << "ID: " << id << std::endl;
  });

  //detail_client.GetBook("1", handler);
  //detail_client.UpdateBook("1", "1Q84", 32.1, handler);
  //detail_client.GetBook("1", handler);
  //detail_client.DeleteBook("1", handler);

  //list_client.ListBooks(handler);

  io_context.run();


  return 0;
}
