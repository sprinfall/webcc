#ifndef EXAMPLE_COMMON_BOOK_JSON_H_
#define EXAMPLE_COMMON_BOOK_JSON_H_

#include <string>

#include "json/json-forwards.h"

struct Book;

std::string JsonToString(const Json::Value& json);

Json::Value StringToJson(const std::string& str);

Json::Value BookToJson(const Book& book);

std::string BookToJsonString(const Book& book);
bool JsonStringToBook(const std::string& json_str, Book* book);

#endif  // EXAMPLE_COMMON_BOOK_JSON_H_
