#ifndef EXAMPLE_SOAP_BOOK_SERVER_BOOK_SERVICE_H_
#define EXAMPLE_SOAP_BOOK_SERVER_BOOK_SERVICE_H_

#include "webcc/soap_service.h"

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

#endif  // EXAMPLE_SOAP_BOOK_SERVER_BOOK_SERVICE_H_
