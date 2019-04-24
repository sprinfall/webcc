#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <vector>

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/rest_server.h"
#include "webcc/rest_service.h"

#include "examples/common/book.h"
#include "examples/common/book_json.h"

#if (defined(WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

// -----------------------------------------------------------------------------

static BookStore g_book_store;

static void Sleep(int seconds) {
  if (seconds > 0) {
    LOG_INFO("Sleep %d seconds...", seconds);
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }
}

// -----------------------------------------------------------------------------

class BookListService : public webcc::RestListService {
public:
  explicit BookListService(int sleep_seconds)
      : sleep_seconds_(sleep_seconds) {
  }

protected:
  // Get a list of books based on query parameters.
  void Get(const webcc::UrlQuery& query, webcc::RestResponse* response) override;

  // Create a new book.
  void Post(const std::string& request_content,
            webcc::RestResponse* response) override;

private:
  // Sleep some seconds before send back the response.
  // For testing timeout control in client side.
  int sleep_seconds_;
};

// -----------------------------------------------------------------------------

// The URL is like '/books/{BookID}', and the 'url_matches' parameter
// contains the matched book ID.
class BookDetailService : public webcc::RestDetailService {
public:
  explicit BookDetailService(int sleep_seconds)
      : sleep_seconds_(sleep_seconds) {
  }

protected:
  // Get the detailed information of a book.
  void Get(const webcc::UrlMatches& url_matches,
           const webcc::UrlQuery& query,
           webcc::RestResponse* response) override;

  // Update a book.
  void Put(const webcc::UrlMatches& url_matches,
           const std::string& request_content,
           webcc::RestResponse* response) override;

  // Delete a book.
  void Delete(const webcc::UrlMatches& url_matches,
              webcc::RestResponse* response) override;

private:
  // Sleep some seconds before send back the response.
  // For testing timeout control in client side.
  int sleep_seconds_;
};

// -----------------------------------------------------------------------------

// Return all books as a JSON array.
void BookListService::Get(const webcc::UrlQuery& /*query*/,
                          webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  Json::Value json(Json::arrayValue);

  for (const Book& book : g_book_store.books()) {
    json.append(BookToJson(book));
  }

  // TODO: Simplify
  response->content = JsonToString(json);
  response->media_type = webcc::media_types::kApplicationJson;
  response->charset = "utf-8";
  response->status = webcc::Status::kOK;
}

void BookListService::Post(const std::string& request_content,
                           webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  Book book;
  if (JsonStringToBook(request_content, &book)) {
    std::string id = g_book_store.AddBook(book);

    Json::Value json;
    json["id"] = id;

    response->content = JsonToString(json);
    response->media_type = webcc::media_types::kApplicationJson;
    response->charset = "utf-8";
    response->status = webcc::Status::kCreated;
  } else {
    // Invalid JSON
    response->status = webcc::Status::kBadRequest;
  }
}

// -----------------------------------------------------------------------------

void BookDetailService::Get(const webcc::UrlMatches& url_matches,
                            const webcc::UrlQuery& query,
                            webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_matches.size() != 1) {
    // Using kNotFound means the resource specified by the URL cannot be found.
    // kBadRequest could be another choice.
    response->status = webcc::Status::kNotFound;
    return;
  }

  const std::string& book_id = url_matches[0];

  const Book& book = g_book_store.GetBook(book_id);
  if (book.IsNull()) {
    response->status = webcc::Status::kNotFound;
    return;
  }

  response->content = BookToJsonString(book);
  response->media_type = webcc::media_types::kApplicationJson;
  response->charset = "utf-8";
  response->status = webcc::Status::kOK;
}

void BookDetailService::Put(const webcc::UrlMatches& url_matches,
                            const std::string& request_content,
                            webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_matches.size() != 1) {
    response->status = webcc::Status::kNotFound;
    return;
  }

  const std::string& book_id = url_matches[0];

  Book book;
  if (!JsonStringToBook(request_content, &book)) {
    response->status = webcc::Status::kBadRequest;
    return;
  }

  book.id = book_id;
  g_book_store.UpdateBook(book);

  response->status = webcc::Status::kOK;
}

void BookDetailService::Delete(const webcc::UrlMatches& url_matches,
                               webcc::RestResponse* response) {
  Sleep(sleep_seconds_);

  if (url_matches.size() != 1) {
    response->status = webcc::Status::kNotFound;
    return;
  }

  const std::string& book_id = url_matches[0];

  if (!g_book_store.DeleteBook(book_id)) {
    response->status = webcc::Status::kNotFound;
    return;
  }

  response->status = webcc::Status::kOK;
}

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port> [seconds]" << std::endl;
  std::cout << "If |seconds| is provided, the server will sleep these seconds "
               "before sending back each response."
            << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
  std::cout << "    " << argv0 << " 8080 3" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  int sleep_seconds = 0;
  if (argc >= 3) {
    sleep_seconds = std::atoi(argv[2]);
  }

  std::size_t workers = 2;

  try {
    webcc::RestServer server(port, workers);

    server.Bind(std::make_shared<BookListService>(sleep_seconds),
                "/books", false);

    server.Bind(std::make_shared<BookDetailService>(sleep_seconds),
                "/books/(\\d+)", true);

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
