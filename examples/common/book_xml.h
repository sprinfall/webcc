#ifndef EXAMPLE_COMMON_BOOK_XML_H_
#define EXAMPLE_COMMON_BOOK_XML_H_

#include <list>
#include <string>

#include "pugixml/pugixml.hpp"

struct Book;

// -----------------------------------------------------------------------------

// Convert the following XML node to a book object.
//   <book>
//     <id>1</id>
//     <title>1984</title>
//     <price>12.3</price>
//   </book>
bool XmlToBook(pugi::xml_node xbook, Book* book);

// Convert a book object to XML and append to the given parent.
void BookToXml(const Book& book, pugi::xml_node* xparent);

// Convert the following XML node to a list of book objects.
//   <books>
//     <book>
//       <id>1</id>
//       <title>1984</title>
//       <price>12.3</price>
//     </book>
//     ...
//   </books>
bool XmlToBookList(pugi::xml_node xbooks, std::list<Book>* books);

// Convert a list of book objects to XML and append to the given parent.
void BookListToXml(const std::list<Book>& books, pugi::xml_node* xparent);

// Convert the following XML string to a book object.
//   <book>
//     <id>1</id>
//     <title>1984</title>
//     <price>12.3</price>
//   </book>
bool XmlStringToBook(const std::string& xml_string, Book* book);

// Convert a book object to XML string.
std::string BookToXmlString(const Book& book, bool format_raw = true,
                            const char* indent = "");

// -----------------------------------------------------------------------------

// This example defines its own result XML which will be embedded into the SOAP
// envolope as CDATA. The general schema of this result XML is:
//   <webcc type = "result">
//       <status code = "{code}" message = "{message}">
//   </webcc>
// The "status" node is mandatory, you should define proper status codes and
// messages according to your needs.
// Additional data is attached as the sibling of "status" node, e.g.,
//   <webcc type = "result">
//     <status code = "{code}" message = "{message}">
//     <book>
//       <id>{book.id}</id>
//       <title>{book.title}</title>
//       <price>{book.price}</price>
//     </book>
//   </webcc>

// Create a result XML as below:
//   <webcc type = "result">
//     <status code = "{code}" message = "{message}">
//   </webcc>
std::string NewResultXml(int code, const char* message);

// Create a result XML as below:
//   <webcc type = "result">
//     <status code = "{code}" message = "{message}">
//     <{node}>
//       <{key}>{value}</{key}>
//     </{node}>
//   </webcc>
std::string NewResultXml(int code, const char* message, const char* node,
                         const char* key, const char* value);

// Create a result XML as below:
//   <webcc type = "result">
//     <status code = "{code}" message = "{message}">
//     <book>
//       <id>{book.id}</id>
//       <title>{book.title}</title>
//       <price>{book.price}</price>
//     </book>
//   </webcc>
std::string NewResultXml(int code, const char* message, const Book& book);

// Create a result XML as below:
//   <webcc type = "result">
//     <status code = "{code}" message = "{message}">
//     <books>
//       <book>
//         <id>{book.id}</id>
//         <title>{book.title}</title>
//         <price>{book.price}</price>
//       </book>
//       ...
//     </books>
//   </webcc>
std::string NewResultXml(int code, const char* message,
                         const std::list<Book>& books);

#endif  // EXAMPLE_COMMON_BOOK_XML_H_
