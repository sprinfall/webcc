#include <functional>
#include <iostream>
#include <string>

#include "pugixml/pugixml.hpp"

#include "webcc/logger.h"
#include "webcc/soap_client.h"

#include "examples/common/book.h"
#include "examples/common/book_xml.h"

#if (defined(WIN32) || defined(_WIN64))
#if defined(_DEBUG) && defined(WEBCC_ENABLE_VLD)
#pragma message ("< include vld.h >")
#include "vld/vld.h"
#pragma comment(lib, "vld")
#endif
#endif

// -----------------------------------------------------------------------------

static const std::string kResult = "Result";

static void PrintSeparateLine() {
  std::cout << "--------------------------------";
  std::cout << "--------------------------------";
  std::cout << std::endl;
}

// -----------------------------------------------------------------------------

class BookClient {
public:
  explicit BookClient(const std::string& url);
  
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

// -----------------------------------------------------------------------------

BookClient::BookClient(const std::string& url)
    : soap_client_(url), code_(0) {
  soap_client_.set_service_ns({ "ser", "http://www.example.com/book/" });

  // Customize response XML format.
  soap_client_.set_format_raw(false);
  soap_client_.set_indent_str("  ");
}

bool BookClient::CreateBook(const std::string& title, double price,
                            std::string* id) {
  PrintSeparateLine();
  std::cout << "CreateBook: " << title << ", " << price << std::endl;

  webcc::SoapParameter parameter{
    "book",
    BookToXmlString({ "", title, price }),
    true,  // as_cdata
  };
  std::string result_xml;
  if (!Call1("CreateBook", std::move(parameter), &result_xml)) {
    return false;
  }

  auto callback = [id](pugi::xml_node xnode) {
    *id = xnode.child("book").child("id").text().as_string();
    return !id->empty();
  };
  return ParseResultXml(result_xml, callback);
}

bool BookClient::GetBook(const std::string& id, Book* book) {
  PrintSeparateLine();
  std::cout << "GetBook: " << id << std::endl;

  std::string result_xml;
  if (!Call1("GetBook", { "id", id }, &result_xml)) {
    return false;
  }

  auto callback = [book](pugi::xml_node xnode) {
    return XmlToBook(xnode.child("book"), book);
  };
  return ParseResultXml(result_xml, callback);
}

bool BookClient::ListBooks(std::list<Book>* books) {
  PrintSeparateLine();
  std::cout << "ListBooks" << std::endl;

  std::string result_xml;
  if (!Call0("ListBooks", &result_xml)) {
    return false;
  }

  auto callback = [books](pugi::xml_node xnode) {
    return XmlToBookList(xnode.child("books"), books);
  };
  return ParseResultXml(result_xml, callback);
}

bool BookClient::DeleteBook(const std::string& id) {
  PrintSeparateLine();
  std::cout << "DeleteBook: " << id << std::endl;

  std::string result_xml;
  if (!Call1("DeleteBook", { "id", id }, &result_xml)) {
    return false;
  }

  return ParseResultXml(result_xml, {});
}

bool BookClient::Call0(const std::string& operation, std::string* result_str) {
  return Call(operation, {}, result_str);
}

bool BookClient::Call1(const std::string& operation,
                       webcc::SoapParameter&& parameter,
                       std::string* result_str) {
  std::vector<webcc::SoapParameter> parameters{
    { std::move(parameter) }
  };
  return Call(operation, std::move(parameters), result_str);
}

bool BookClient::Call(const std::string& operation,
                      std::vector<webcc::SoapParameter>&& parameters,
                      std::string* result_str) {
  if (!soap_client_.Request(operation, std::move(parameters), kResult, 0,
                            result_str)) {
    PrintError();
    return false;
  }
  return true;
}

void BookClient::PrintError() {
  std::cout << webcc::DescribeError(soap_client_.error());
  if (soap_client_.timed_out()) {
    std::cout << " (timed out)";
  }
  std::cout << std::endl;
}

bool BookClient::ParseResultXml(const std::string& result_xml,
                                std::function<bool(pugi::xml_node)> callback) {
  pugi::xml_document xdoc;
  if (!xdoc.load_string(result_xml.c_str())) {
    return false;
  }

  pugi::xml_node xwebcc = xdoc.document_element();

  pugi::xml_node xstatus = xwebcc.child("status");
  code_ = xstatus.attribute("code").as_int();
  message_ = xstatus.attribute("message").as_string();

  if (callback) {
    return callback(xwebcc);
  }

  return true;
}

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <url>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " http://localhost:8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::string url = argv[1];

  BookClient client(url + "/book");

  std::string id1;
  if (!client.CreateBook("1984", 12.3, &id1)) {
    std::cerr << "Failed to create book." << std::endl;
    return 2;
  }

  std::cout << "Book ID: " << id1 << std::endl;

  std::string id2;
  if (!client.CreateBook("1Q84", 32.1, &id2)) {
    std::cerr << "Failed to create book." << std::endl;
    return 2;
  }

  std::cout << "Book ID: " << id2 << std::endl;

  Book book;
  if (!client.GetBook(id1, &book)) {
    std::cerr << "Failed to get book." << std::endl;
    return 2;
  }

  std::cout << "Book: " << book << std::endl;

  std::list<Book> books;
  if (!client.ListBooks(&books)) {
    std::cerr << "Failed to list books." << std::endl;
    return 2;
  }

  for (const Book& book : books) {
    std::cout << "Book: " << book << std::endl;
  }

  if (client.DeleteBook(id1)) {
    std::cout << "Book deleted: " << id1 << std::endl;
  }

  return 0;
}
