#include <iostream>
#include <list>
#include <sstream>

#include "webcc/logger.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"
#include "webcc/soap_server.h"
#include "webcc/soap_service.h"

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

static BookStore g_book_store;

static const std::string kResult = "Result";

// -----------------------------------------------------------------------------

class BookService : public webcc::SoapService {
public:
  bool Handle(const webcc::SoapRequest& soap_request,
              webcc::SoapResponse* soap_response) override;

private:
  bool CreateBook(const webcc::SoapRequest& soap_request,
                  webcc::SoapResponse* soap_response);

  bool GetBook(const webcc::SoapRequest& soap_request,
               webcc::SoapResponse* soap_response);

  bool ListBooks(const webcc::SoapRequest& soap_request,
                 webcc::SoapResponse* soap_response);

  bool DeleteBook(const webcc::SoapRequest& soap_request,
                  webcc::SoapResponse* soap_response);
};

// -----------------------------------------------------------------------------

bool BookService::Handle(const webcc::SoapRequest& soap_request,
                         webcc::SoapResponse* soap_response) {
  const std::string& operation = soap_request.operation();

  soap_response->set_service_ns({
    "ser",
    "http://www.example.com/book/"
  });

  soap_response->set_operation(operation);

  if (operation == "CreateBook") {
    return CreateBook(soap_request, soap_response);

  } else if (operation == "GetBook") {
    return GetBook(soap_request, soap_response);

  } else if (operation == "ListBooks") {
    return ListBooks(soap_request, soap_response);

  } else if (operation == "DeleteBook") {
    return DeleteBook(soap_request, soap_response);

  } else {
    LOG_ERRO("Operation '%s' is not supported.", operation.c_str());
    return false;
  }

  return false;
}

bool BookService::CreateBook(const webcc::SoapRequest& soap_request,
                             webcc::SoapResponse* soap_response) {
  // Request SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:CreateBook xmlns:ser="..." />
  //         <ser:book>
  //           <![CDATA[
  //           <book>
  //             <title>1984</title>
  //             <price>12.3</price>
  //           </book>
  //           ]]>
  //         </ser:book>
  //       </ser:CreateBook>
  //     </soap:Body>
  //   </soap:Envelope>

  // Response SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:CreateBookResponse xmlns:ser="...">
  //         <ser:Result>
  //           <![CDATA[
  //           <webcc type = "response">
  //             <status code = "0" message = "ok">
  //             <book>
  //               <id>1</id>
  //             </book>
  //           </webcc>
  //           ]]>
  //         </ser:Result>
  //       </ser:CreateBookResponse>
  //     </soap:Body>
  //   </soap:Envelope>

  const std::string& title = soap_request.GetParameter("title");

  const std::string& book_xml = soap_request.GetParameter("book");

  Book book;
  XmlStringToBook(book_xml, &book);  // TODO: Error handling

  std::string id = g_book_store.AddBook(book);

  std::string response_xml = NewResultXml(0, "ok", "book", "id",
                                          id.c_str());

  soap_response->set_simple_result(kResult, std::move(response_xml), true);

  return true;
}

bool BookService::GetBook(const webcc::SoapRequest& soap_request,
                          webcc::SoapResponse* soap_response) {
  // Request SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:GetBook xmlns:ser="..." />
  //         <ser:id>1</ser:id>
  //       </ser:GetBook>
  //     </soap:Body>
  //   </soap:Envelope>

  // Response SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:GetBookResponse xmlns:ser="...">
  //         <ser:Result>
  //           <![CDATA[
  //           <webcc type = "response">
  //             <status code = "0" message = "ok">
  //             <book>
  //               <id>1</id>
  //               <title>1984</title>
  //               <price>12.3</price>
  //             </book>
  //           </webcc>
  //           ]]>
  //         </ser:Result>
  //       </ser:GetBookResponse>
  //     </soap:Body>
  //   </soap:Envelope>

  const std::string& id = soap_request.GetParameter("id");

  const Book& book = g_book_store.GetBook(id);

  soap_response->set_simple_result(kResult, NewResultXml(0, "ok", book), true);

  return true;
}

bool BookService::ListBooks(const webcc::SoapRequest& soap_request,
                            webcc::SoapResponse* soap_response) {
  // Request SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:ListBooks xmlns:ser="..." />
  //     </soap:Body>
  //   </soap:Envelope>

  // Response SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:ListBooksResponse xmlns:ser="...">
  //         <ser:Result>
  //           <![CDATA[
  //           <webcc type = "response">
  //             <status code = "0" message = "ok">
  //             <books>
  //               <book>
  //                 <id>1</id>
  //                 <title>1984</title>
  //                 <price>12.3</price>
  //               </book>
  //               ...
  //             </books>
  //           </webcc>
  //           ]]>
  //         </ser:Result>
  //       </ser:ListBooksResponse>
  //     </soap:Body>
  //   </soap:Envelope>

  const std::list<Book>& books = g_book_store.books();

  soap_response->set_simple_result(kResult, NewResultXml(0, "ok", books), true);

  return true;
}

bool BookService::DeleteBook(const webcc::SoapRequest& soap_request,
                             webcc::SoapResponse* soap_response) {
  // Request SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:DeleteBook xmlns:ser="..." />
  //         <ser:id>1</ser:id>
  //       </ser:DeleteBook>
  //     </soap:Body>
  //   </soap:Envelope>

  // Response SOAP envelope:
  //   <soap:Envelope xmlns:soap="...">
  //     <soap:Body>
  //       <ser:DeleteBookResponse xmlns:ser="...">
  //         <ser:Result>
  //           <![CDATA[
  //           <webcc type = "response">
  //             <status code = "0" message = "ok">
  //           </webcc>
  //           ]]>
  //         </ser:Result>
  //       </ser:DeleteBookResponse>
  //     </soap:Body>
  //   </soap:Envelope>

  const std::string& id = soap_request.GetParameter("id");

  if (g_book_store.DeleteBook(id)) {
    soap_response->set_simple_result(kResult, NewResultXml(0, "ok"), true);
  } else {
    soap_response->set_simple_result(kResult, NewResultXml(1, "error"), true);
  }

  return true;
}

// -----------------------------------------------------------------------------

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <port>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << " 8080" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::uint16_t port = static_cast<std::uint16_t>(std::atoi(argv[1]));
  std::size_t workers = 2;

  try {
    webcc::SoapServer server(port, workers);

    // Customize response XML format.
    server.set_format_raw(false);
    server.set_indent_str("  ");

    server.Bind(std::make_shared<BookService>(), "/book");
    server.Run();
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
