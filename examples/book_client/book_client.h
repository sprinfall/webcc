#ifndef BOOK_CLIENT_H_
#define BOOK_CLIENT_H_

#include <filesystem>
#include <list>
#include <string>

#include "boost/filesystem/path.hpp"
#include "json/json-forwards.h"

#include "webcc/client_session.h"

#include "book.h"

class BookClient {
public:
  explicit BookClient(const std::string& url, int timeout = 0);

  ~BookClient() = default;

  bool Query(std::list<Book>* books);

  bool Create(const std::string& title, double price, std::string* id);

  bool Get(const std::string& id, Book* book);

  bool Set(const std::string& id, const std::string& title, double price);

  bool Delete(const std::string& id);

  // Get photo, save to the given path.
  bool GetPhoto(const std::string& id, const std::filesystem::path& path);

  // Set photo using the file of the given path.
  bool SetPhoto(const std::string& id, const std::filesystem::path& path);

private:
  bool CheckPhoto(const std::filesystem::path& photo);

  // Check HTTP response status.
  bool CheckStatus(webcc::ResponsePtr response, int expected_status);

private:
  std::string url_;
  webcc::ClientSession session_;
};

#endif  // BOOK_CLIENT_H_
