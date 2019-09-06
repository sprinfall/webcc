#ifndef VIEWS_H_
#define VIEWS_H_

#include "webcc/view.h"

#include "boost/filesystem/path.hpp"

namespace bfs = boost::filesystem;

// -----------------------------------------------------------------------------

// URL: /books
class BookListView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override;

private:
  // Get a list of books based on query parameters.
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // Create a new book.
  webcc::ResponsePtr Post(webcc::RequestPtr request);
};

// -----------------------------------------------------------------------------

// URL: /books/{id}
class BookDetailView : public webcc::View {
public:
  explicit BookDetailView(bfs::path photo_dir);

  webcc::ResponsePtr Handle(webcc::RequestPtr request) override;

private:
  // Get the detailed information of a book.
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // Update a book.
  webcc::ResponsePtr Put(webcc::RequestPtr request);

  // Delete a book.
  webcc::ResponsePtr Delete(webcc::RequestPtr request);

private:
  bfs::path photo_dir_;
};

// -----------------------------------------------------------------------------

// URL: /books/{id}/photo
class BookPhotoView : public webcc::View {
public:
  explicit BookPhotoView(bfs::path photo_dir);

  webcc::ResponsePtr Handle(webcc::RequestPtr request) override;

  // Stream the request data, an image, of PUT into a temp file.
  bool Stream(const std::string& method) override {
    return method == "PUT";
  }

private:
  // Get the photo of the book.
  webcc::ResponsePtr Get(webcc::RequestPtr request);

  // Set the photo of the book.
  webcc::ResponsePtr Put(webcc::RequestPtr request);

  // Delete the photo of the book.
  webcc::ResponsePtr Delete(webcc::RequestPtr request);

private:
  bfs::path photo_dir_;
};

#endif  // VIEWS_H_
