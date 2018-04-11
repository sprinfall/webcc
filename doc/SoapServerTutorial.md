# SOAP Server Tutorial

Suppose you want to provide a calculator web service just like the one from [ParaSoft](http://ws1.parasoft.com/glue/calculator.wsdl).

Firstly, create a class `CalcService` which is derived from `webcc::SoapService`, override the `Handle` method:
```cpp
#include "webcc/soap_service.h"

class CalcService : public webcc::SoapService {
public:
  CalcService() = default;

  bool Handle(const webcc::SoapRequest& soap_request,
              webcc::SoapResponse* soap_response) override;
};
```
The `Handle` method has two parameters, one for request (input), one for response (output).
The implementation is quite straightforward:
- Get operation from request.
- Get parameters from request.
- Calculate the result.
- Set namespaces, result name and so on to response.
- Set result to response.

```cpp
#include "calc_service.h"

#include "boost/lexical_cast.hpp"

#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

bool CalcService::Handle(const webcc::SoapRequest& soap_request,
                         webcc::SoapResponse* soap_response) {
  try {
    if (soap_request.operation() == "add") {
      double x = boost::lexical_cast<double>(soap_request.GetParameter("x"));
      double y = boost::lexical_cast<double>(soap_request.GetParameter("y"));

      double result = x + y;

      soap_response->set_soapenv_ns(webcc::kSoapEnvNamespace);
      soap_response->set_service_ns({
        "cal",
        "http://www.example.com/calculator/"
      });
      soap_response->set_operation(soap_request.operation());
      soap_response->set_result_name("Result");
      soap_response->set_result(std::to_string(result));

      return true;

    } else {
      // NOT_IMPLEMENTED
    }
  } catch (boost::bad_lexical_cast&) {
    // BAD_REQUEST
  }

  return false;
}
```

The `main` function would be:
```cpp
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
    std::cout << "  E.g.," << std::endl;
    std::cout << "    " << argv[0] << " 8080" << std::endl;
    return 1;
  }

  unsigned short port = std::atoi(argv[1]);

  // Number of worker threads.
  std::size_t workers = 2;

  try {
    webcc::SoapServer server(port, workers);

    server.RegisterService(std::make_shared<CalcService>(),
                           "/calculator");

    server.Run();

  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```
