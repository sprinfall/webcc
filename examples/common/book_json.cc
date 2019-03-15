#include "examples/common/book_json.h"

#include <sstream>
#include <iostream>

#include "json/json.h"

#include "examples/common/book.h"

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
  Json::Value json;
  json["id"] = book.id;
  json["title"] = book.title;
  json["price"] = book.price;
  return json;
}

Book JsonToBook(const Json::Value& json) {
  return {
    json["id"].asString(),
    json["title"].asString(),
    json["price"].asDouble(),
  };
}

std::string BookToJsonString(const Book& book) {
  return JsonToString(BookToJson(book));
}

bool JsonStringToBook(const std::string& json_str, Book* book) {
  Json::Value json = StringToJson(json_str);

  if (!json) {
    return false;
  }

  *book = JsonToBook(json);
  return true;
}
