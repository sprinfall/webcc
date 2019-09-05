#include <iostream>
#include <string>

#include "boost/filesystem/operations.hpp"

#include "json/json.h"

#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

#include "examples/common/book.h"
#include "examples/common/book_json.h"

#if (defined(_WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

namespace bfs = boost::filesystem;

// -----------------------------------------------------------------------------

static BookStore g_book_store;

// -----------------------------------------------------------------------------
// BookListView

// URL: /books
class BookListView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return Get(request);
    }

    if (request->method() == "POST") {
      return Post(request);
    }

    return {};
  }

private:
  // Get a list of books based on query parameters.
  webcc::ResponsePtr Get(webcc::RequestPtr request) {
    Json::Value json(Json::arrayValue);

    for (const Book& book : g_book_store.books()) {
      json.append(BookToJson(book));
    }

    // Return all books as a JSON array.

    return webcc::ResponseBuilder{}.OK().Body(JsonToString(json)).Json().Utf8()();
  }

  // Create a new book.
  webcc::ResponsePtr Post(webcc::RequestPtr request) {
    Book book;
    if (JsonStringToBook(request->data(), &book)) {
      std::string id = g_book_store.AddBook(book);

      Json::Value json;
      json["id"] = id;

      return webcc::ResponseBuilder{}.Created().Body(JsonToString(json)).Json().Utf8()();
    } else {
      // Invalid JSON
      return webcc::ResponseBuilder{}.BadRequest()();
    }
  }
};

// -----------------------------------------------------------------------------
// BookDetailView

// URL: /books/{id}
class BookDetailView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return Get(request);
    }
    if (request->method() == "PUT") {
      return Put(request);
    }
    if (request->method() == "DELETE") {
      return Delete(request);
    }
    return {};
  }

private:
  // Get the detailed information of a book.
  webcc::ResponsePtr Get(webcc::RequestPtr request) {
    if (request->args().size() != 1) {
      // NotFound means the resource specified by the URL cannot be found.
      // BadRequest could be another choice.
      return webcc::ResponseBuilder{}.NotFound()();
    }

    const std::string& id = request->args()[0];

    const Book& book = g_book_store.GetBook(id);
    if (book.IsNull()) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    return webcc::ResponseBuilder{}.OK().Body(BookToJsonString(book)).Json().Utf8()();
  }

  // Update a book.
  webcc::ResponsePtr Put(webcc::RequestPtr request) {
    if (request->args().size() != 1) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    const std::string& id = request->args()[0];

    Book book;
    if (!JsonStringToBook(request->data(), &book)) {
      return webcc::ResponseBuilder{}.BadRequest()();
    }

    book.id = id;
    g_book_store.UpdateBook(book);

    return webcc::ResponseBuilder{}.OK()();
  }

  // Delete a book.
  webcc::ResponsePtr Delete(webcc::RequestPtr request) {
    if (request->args().size() != 1) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    const std::string& id = request->args()[0];

    if (!g_book_store.DeleteBook(id)) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    return webcc::ResponseBuilder{}.OK()();
  }
};

// -----------------------------------------------------------------------------
// BookPhotoView

// URL: /books/{id}/photo
class BookPhotoView : public webcc::View {
public:
  explicit BookPhotoView(bfs::path upload_dir)
      : upload_dir_(std::move(upload_dir)) {
  }

  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return Get(request);
    }

    if (request->method() == "PUT") {
      return Put(request);
    }

    if (request->method() == "DELETE") {
      return Delete(request);
    }

    return {};
  }

  // Stream the request data, an image, of PUT into a temp file.
  bool Stream(const std::string& method) override {
    return method == "PUT";
  }

private:
  // Get the photo of the book.
  // TODO: Check content type to see if it's JPEG.
  webcc::ResponsePtr Get(webcc::RequestPtr request) {
    if (request->args().size() != 1) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    const std::string& id = request->args()[0];
    const Book& book = g_book_store.GetBook(id);
    if (book.IsNull()) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    bfs::path photo_path = GetPhotoPath(id);
    if (!bfs::exists(photo_path)) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    // File() might throw Error::kFileError.
    // TODO: Avoid exception handling.
    try {
      return webcc::ResponseBuilder{}.OK().File(photo_path)();
    } catch (const webcc::Error&) {
      return webcc::ResponseBuilder{}.NotFound()();
    }
  }

  // Set the photo of the book.
  // TODO: Check content type to see if it's JPEG.
  webcc::ResponsePtr Put(webcc::RequestPtr request) {
    if (request->args().size() != 1) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    const std::string& id = request->args()[0];

    const Book& book = g_book_store.GetBook(id);
    if (book.IsNull()) {
      return webcc::ResponseBuilder{}.NotFound()();
    }

    request->file_body()->Move(GetPhotoPath(id));

    return webcc::ResponseBuilder{}.OK()();
  }

  // Delete the photo of the book.
  webcc::ResponsePtr Delete(webcc::RequestPtr request) {
    return {};
  }

private:
  bfs::path GetPhotoPath(const std::string& book_id) const {
    return upload_dir_ / "book_photo" / (book_id + ".jpg");
  }

private:
  bfs::path upload_dir_;
};

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "usage: rest_book_server <port> <upload_dir>" << std::endl;
    std::cout << "examples:" << std::endl;
    std::cout << "  $ rest_book_server 8080 D:/upload" << std::endl;
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));

  bfs::path upload_dir = argv[2];
  if (!bfs::is_directory(upload_dir) || !bfs::exists(upload_dir)) {
    std::cerr << "Invalid upload dir!" << std::endl;
    return 1;
  }

  try {
    webcc::Server server(port);  // No doc root

    server.Route("/books",
                 std::make_shared<BookListView>(),
                 { "GET", "POST" });

    server.Route(webcc::R("/books/(\\d+)"),
                 std::make_shared<BookDetailView>(),
                 { "GET", "PUT", "DELETE" });

    server.Route(webcc::R("/books/(\\d+)/photo"),
                 std::make_shared<BookPhotoView>(upload_dir),
                 { "GET", "PUT", "DELETE" });

    server.Run(2);

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
