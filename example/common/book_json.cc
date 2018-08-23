#include "example/common/book_json.h"

#include <sstream>
#include <iostream>

#include "json/json.h"

#include "example/common/book.h"

std::string JsonToString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, json);
}

Json::Value StringToJson(const std::string& str) {
  Json::Value json;

  Json::CharReaderBuilder builder;
  std::stringstream stream(str);
  std::string errs;
  if (!Json::parseFromStream(builder, stream, &json, &errs)) {
    std::cerr << errs << std::endl;
  }

  return json;
}

Json::Value BookToJson(const Book& book) {
  Json::Value root;
  root["id"] = book.id;
  root["title"] = book.title;
  root["price"] = book.price;
  return root;
}

std::string BookToJsonString(const Book& book) {
  return JsonToString(BookToJson(book));
}

bool JsonStringToBook(const std::string& json_str, Book* book) {
  Json::Value json = StringToJson(json_str);

  if (!json) {
    return false;
  }

  book->id = json["id"].asString();
  book->title = json["title"].asString();
  book->price = json["price"].asDouble();

  return true;
}
