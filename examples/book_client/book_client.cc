#include "book_client.h"

#include <iostream>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem/operations.hpp"
#include "json/json.h"

#include "book_json.h"

BookClient::BookClient(const std::string& url, int timeout)
    : url_(url), session_(timeout) {
  // Default Content-Type for requests who have a body.
  session_.set_media_type("application/json");
  session_.set_charset("utf-8");
}

bool BookClient::Query(std::list<Book>* books) {
  try {
    auto r = session_.Send(WEBCC_GET(url_).Path("books")());

    if (!CheckStatus(r, webcc::Status::kOK)) {
      // Response HTTP status error.
      return false;
    }

    Json::Value json = StringToJson(r->data());

    if (!json.isArray()) {
      return false;  // Should be a JSON array of books.
    }

    for (Json::ArrayIndex i = 0; i < json.size(); ++i) {
      books->push_back(JsonToBook(json[i]));
    }

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::Create(const std::string& title, double price,
                        std::string* id) {
  Json::Value req_json;
  req_json["title"] = title;
  req_json["price"] = price;

  try {
    auto r = session_.Send(WEBCC_POST(url_).Path("books").
                           Body(JsonToString(req_json))());

    if (!CheckStatus(r, webcc::Status::kCreated)) {
      return false;
    }

    Json::Value rsp_json = StringToJson(r->data());
    *id = rsp_json["id"].asString();

    if (id->empty()) {
      return false;
    }

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::Get(const std::string& id, Book* book) {
  try {
    auto r = session_.Send(WEBCC_GET(url_).Path("books").Path(id)());

    if (!CheckStatus(r, webcc::Status::kOK)) {
      return false;
    }

    return JsonStringToBook(r->data(), book);

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::Set(const std::string& id, const std::string& title,
                     double price) {
  Json::Value json;
  json["title"] = title;
  json["price"] = price;

  try {
    auto r = session_.Send(WEBCC_PUT(url_).Path("books").Path(id).
                           Body(JsonToString(json))());

    if (!CheckStatus(r, webcc::Status::kOK)) {
      return false;
    }

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::Delete(const std::string& id) {
  try {
    auto r = session_.Send(WEBCC_DELETE(url_).Path("books").Path(id)());

    if (!CheckStatus(r, webcc::Status::kOK)) {
      return false;
    }

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::GetPhoto(const std::string& id, const bfs::path& path) {
  try {
    auto r = session_.Send(WEBCC_GET(url_).
                           Path("books").Path(id).Path("photo")(),
                           true);  // Save to temp file

    if (!CheckStatus(r, webcc::Status::kOK)) {
      return false;
    }

    r->file_body()->Move(path);

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::SetPhoto(const std::string& id, const bfs::path& path) {
  try {
    if (!CheckPhoto(path)) {
      return false;
    }

    auto r = session_.Send(WEBCC_PUT(url_).
                           Path("books").Path(id).Path("photo").
                           File(path)());

    if (!CheckStatus(r, webcc::Status::kOK)) {
      return false;
    }

    return true;

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return false;
  }
}

bool BookClient::CheckPhoto(const bfs::path& photo) {
  if (photo.empty()) {
    return false;
  }

  if (!bfs::is_regular_file(photo) || !bfs::exists(photo)) {
    return false;
  }

  auto ext = photo.extension().string();
  if (!boost::iequals(ext, ".jpg") && !boost::iequals(ext, ".jpeg")) {
    return false;
  }

  return true;
}

bool BookClient::CheckStatus(webcc::ResponsePtr response, int expected_status) {
  if (response->status() != expected_status) {
    std::cerr << "HTTP status error (actual: " << response->status()
              << "expected: " << expected_status << ")." << std::endl;
    return false;
  }
  return true;
}
