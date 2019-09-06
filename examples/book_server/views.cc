#include "views.h"

#include "json/json.h"

#include "boost/filesystem/operations.hpp"
#include "webcc/response_builder.h"

#include "book.h"
#include "book_db.h"
#include "book_json.h"

// -----------------------------------------------------------------------------

static BookDB g_book_db;

// -----------------------------------------------------------------------------

webcc::ResponsePtr BookListView::Handle(webcc::RequestPtr request) {
  if (request->method() == "GET") {
    return Get(request);
  }
  if (request->method() == "POST") {
    return Post(request);
  }
  return {};
}

webcc::ResponsePtr BookListView::Get(webcc::RequestPtr request) {
  Json::Value json(Json::arrayValue);

  for (const Book& book : g_book_db.books()) {
    json.append(BookToJson(book));
  }

  // Return all books as a JSON array.

  return webcc::ResponseBuilder{}.OK().Body(JsonToString(json)).Json().Utf8()();
}

webcc::ResponsePtr BookListView::Post(webcc::RequestPtr request) {
  Book book;
  if (JsonStringToBook(request->data(), &book)) {
    std::string id = g_book_db.Add(book);

    Json::Value json;
    json["id"] = id;

    return webcc::ResponseBuilder{}.Created().Body(JsonToString(json)).Json().Utf8()();
  } else {
    // Invalid JSON
    return webcc::ResponseBuilder{}.BadRequest()();
  }
}

// -----------------------------------------------------------------------------

BookDetailView::BookDetailView(bfs::path photo_dir)
    : photo_dir_(std::move(photo_dir)) {
}

webcc::ResponsePtr BookDetailView::Handle(webcc::RequestPtr request) {
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

webcc::ResponsePtr BookDetailView::Get(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    // NotFound means the resource specified by the URL cannot be found.
    // BadRequest could be another choice.
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  const Book& book = g_book_db.Get(id);
  if (book.IsNull()) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  return webcc::ResponseBuilder{}.OK().Body(BookToJsonString(book)).Json().Utf8()();
}

webcc::ResponsePtr BookDetailView::Put(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  Book book;
  if (!JsonStringToBook(request->data(), &book)) {
    return webcc::ResponseBuilder{}.BadRequest()();
  }

  book.id = id;
  g_book_db.Set(book);

  return webcc::ResponseBuilder{}.OK()();
}

webcc::ResponsePtr BookDetailView::Delete(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  std::string photo_name = g_book_db.GetPhoto(id);

  // Delete the book from DB.
  if (!g_book_db.Delete(id)) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  // Delete the photo from file system.
  if (!photo_name.empty()) {
    boost::system::error_code ec;
    bfs::remove(photo_dir_ / photo_name, ec);
  }

  return webcc::ResponseBuilder{}.OK()();
}

// -----------------------------------------------------------------------------

BookPhotoView::BookPhotoView(bfs::path photo_dir)
    : photo_dir_(std::move(photo_dir)) {
}

webcc::ResponsePtr BookPhotoView::Handle(webcc::RequestPtr request) {
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

// TODO: Check content type to see if it's JPEG.
webcc::ResponsePtr BookPhotoView::Get(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  const Book& book = g_book_db.Get(id);
  if (book.IsNull() || book.photo.empty()) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  bfs::path photo_path = photo_dir_ / book.photo;
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

webcc::ResponsePtr BookPhotoView::Put(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  const Book& book = g_book_db.Get(id);
  if (book.IsNull()) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  // Use ID as the name of the photo.
  // You can also use a UUID or any unique string as the name.
  auto photo_name = id + ".jpg";

  if (!request->file_body()->Move(photo_dir_ / photo_name)) {
    return webcc::ResponseBuilder{}.InternalServerError()();
  }

  // Set photo name to DB.
  if (!g_book_db.SetPhoto(id, photo_name)) {
    return webcc::ResponseBuilder{}.InternalServerError()();
  }

  return webcc::ResponseBuilder{}.OK()();
}

webcc::ResponsePtr BookPhotoView::Delete(webcc::RequestPtr request) {
  if (request->args().size() != 1) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  const std::string& id = request->args()[0];

  const Book& book = g_book_db.Get(id);
  if (book.IsNull() || book.photo.empty()) {
    return webcc::ResponseBuilder{}.NotFound()();
  }

  // Error handling is simplified.
  boost::system::error_code ec;
  bfs::remove(photo_dir_ / book.photo, ec);

  return webcc::ResponseBuilder{}.OK()();
}
