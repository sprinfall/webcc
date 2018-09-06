#ifndef EXAMPLE_SOAP_BOOK_CLIENT_H_
#define EXAMPLE_SOAP_BOOK_CLIENT_H_

#include <functional>
#include <string>

#include "pugixml/pugixml.hpp"
#include "webcc/soap_client.h"

#include "example/common/book.h"

class BookClient {
 public:
  BookClient(const std::string& host, const std::string& port);
  
  int code() const { return code_; }
  const std::string& message() const { return message_; }

  // Create a book.
  bool CreateBook(const std::string& title, double price, std::string* id);

  // Get a book by ID.
  bool GetBook(const std::string& id, Book* book);

  // List all books.
  bool ListBooks(std::list<Book>* books);

  // Delete a book by ID.
  bool DeleteBook(const std::string& id);

 private:
  // Call with 0 parameter.
  bool Call0(const std::string& operation, std::string* result_str);

  // Call with 1 parameter.
  bool Call1(const std::string& operation, webcc::SoapParameter&& parameter,
             std::string* result_str);

  // Simple wrapper of SoapClient::Request() to log error if any.
  bool Call(const std::string& operation,
            std::vector<webcc::SoapParameter>&& parameters,
            std::string* result_str);

  void PrintError();

  bool ParseResultXml(const std::string& result_xml,
                      std::function<bool(pugi::xml_node)> callback);

  webcc::SoapClient soap_client_;

  // Last status.
  int code_;
  std::string message_;
};

#endif  // EXAMPLE_SOAP_BOOK_CLIENT_H_
