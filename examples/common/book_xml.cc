#include "examples/common/book_xml.h"

#include <cassert>
#include <functional>
#include <sstream>

#include "examples/common/book.h"

// -----------------------------------------------------------------------------

// Print a XML node to string.
static std::string PrintXml(pugi::xml_node xnode, bool format_raw = true,
                            const char* indent = "") {
  std::stringstream ss;
  unsigned int flags = format_raw ? pugi::format_raw : pugi::format_indent;
  xnode.print(ss, indent, flags);
  return ss.str();
}

// -----------------------------------------------------------------------------

bool XmlToBook(pugi::xml_node xbook, Book* book) {
  assert(xbook.name() == std::string("book"));

  book->id = xbook.child("id").text().as_string();
  book->title = xbook.child("title").text().as_string();
  book->price = xbook.child("price").text().as_double();

  return true;
}

void BookToXml(const Book& book, pugi::xml_node* xparent) {
  pugi::xml_node xbook = xparent->append_child("book");

  xbook.append_child("id").text().set(book.id.c_str());
  xbook.append_child("title").text().set(book.title.c_str());
  xbook.append_child("price").text().set(book.price);
}

bool XmlToBookList(pugi::xml_node xbooks, std::list<Book>* books) {
  assert(xbooks.name() == std::string("books"));

  pugi::xml_node xbook = xbooks.child("book");

  while (xbook) {
    Book book{
      xbook.child("id").text().as_string(),
      xbook.child("title").text().as_string(),
      xbook.child("price").text().as_double()
    };
    books->push_back(book);

    xbook = xbook.next_sibling("book");
  }

  return true;
}

void BookListToXml(const std::list<Book>& books, pugi::xml_node* xparent) {
  pugi::xml_node xbooks = xparent->append_child("books");

  for (const Book& book : books) {
    BookToXml(book, &xbooks);
  }
}

bool XmlStringToBook(const std::string& xml_string, Book* book) {
  pugi::xml_document xdoc;
  if (!xdoc.load_string(xml_string.c_str())) {
    return false;
  }

  pugi::xml_node xbook = xdoc.document_element();
  if (!xbook) {
    return false;
  }

  if (xbook.name() != std::string("book")) {
    return false;
  }

  return XmlToBook(xbook, book);
}

std::string BookToXmlString(const Book& book, bool format_raw,
                            const char* indent) {
  pugi::xml_document xdoc;
  BookToXml(book, &xdoc);
  return PrintXml(xdoc);
}

// -----------------------------------------------------------------------------

std::string NewRequestXml(const Book& book) {
  pugi::xml_document xdoc;

  pugi::xml_node xwebcc = xdoc.append_child("webcc");
  xwebcc.append_attribute("type") = "request";

  BookToXml(book, &xwebcc);

  return PrintXml(xdoc, false, "  ");
}

// -----------------------------------------------------------------------------

static std::string __NewResultXml(int code, const char* message,
                                  std::function<void(pugi::xml_node*)> callback) {
  pugi::xml_document xdoc;

  pugi::xml_node xwebcc = xdoc.append_child("webcc");
  xwebcc.append_attribute("type") = "response";

  pugi::xml_node xstatus = xwebcc.append_child("status");
  xstatus.append_attribute("code") = code;
  xstatus.append_attribute("message") = message;

  if (callback) {
    callback(&xwebcc);
  }

  return PrintXml(xdoc, false, "  ");
}

std::string NewResultXml(int code, const char* message) {
  return __NewResultXml(code, message, {});
}

std::string NewResultXml(int code, const char* message, const char* node,
                           const char* key, const char* value) {
  auto callback = [node, key, value](pugi::xml_node* xparent) {
    pugi::xml_node xnode = xparent->append_child(node);
    xnode.append_child(key).text() = value;
  };
  return __NewResultXml(code, message, callback);
}

std::string NewResultXml(int code, const char* message, const Book& book) {
  return __NewResultXml(code, message,
                        std::bind(BookToXml, book, std::placeholders::_1));
}

std::string NewResultXml(int code, const char* message,
                           const std::list<Book>& books) {
  return __NewResultXml(code, message,
                        std::bind(BookListToXml, books, std::placeholders::_1));
}
