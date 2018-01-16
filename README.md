# csoap

A lightweight C++ SOAP client & server library based on Boost.Asio.

## Client Usage

Firstly, please install **SoapUI** if you don't have it. We need SoapUI to generate sample requests for each web service operation. The open source version is good enough.

Take the calculator web service provided by ParaSoft as an example. Download the WSDL from http://ws1.parasoft.com/glue/calculator.wsdl, create a SOAP project within SoapUI (remember to check "**Create sample requests for all operations?**"), you will see the sample request for "add" operation as the following:
```xml
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cal="http://www.parasoft.com/wsdl/calculator/">
<soapenv:Header/>
<soapenv:Body>
   <cal:add>
      <cal:x>1</cal:x>
      <cal:y>2</cal:y>
   </cal:add>
</soapenv:Body>
</soapenv:Envelope>
```

In order to call the "add" operation, we have to send a HTTP request with the above SOAP envelope as the content. Let's see how to do this with *csoap*.

Firstly, create a class `CalculatorClient` which is derived from `csoap::SoapClient`:

```cpp
#include <string>
#include "csoap/soap_client.h"

class CalculatorClient : public csoap::SoapClient {
public:
  CalculatorClient() {
    Init();
  }
```

Initialize the URL, host, port, etc. in `Init()`:
```cpp
private:
  void Init() {
    url_ = "/glue/calculator";
    host_ = "ws1.parasoft.com";
    port_ = "";  // Default to "80".
    service_ns_ = { "cal", "http://www.parasoft.com/wsdl/calculator/" };
    result_name_ = "Result";
  }
```
 
Because four calculator operations (*add*, *subtract*, *multiply* and *divide*) all have two parameters, we create a wrapper for `SoapClient::Call()`, name is as `Calc`:
```cpp
bool Calc(const std::string& operation,
          const std::string& x_name,
          const std::string& y_name,
          double x,
          double y,
          double* result) {
  // Prepare parameters.
  std::vector<csoap::Parameter> parameters{
    { x_name, x },
    { y_name, y }
  };

  // Make the call.
  std::string result_str;
  csoap::Error error = Call(operation, std::move(parameters), &result_str);
  
  // Error handling if any.
  if (error != csoap::kNoError) {
    std::cerr << "Error: " << error;
    std::cerr << ", " << csoap::GetErrorMessage(error) << std::endl;
    return false;
  }

  // Convert the result from string to double.
  try {
    *result = boost::lexical_cast<double>(result_str);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  return true;
}
```

Note that the local parameters are moved (with `std::move()`). This is to avoid expensive copy of long string parameters, e.g., XML strings.

Finally, we implement the four operations simply as the following:
```cpp
bool Add(double x, double y, double* result) {
  return Calc("add", "x", "y", x, y, result);
}

bool Subtract(double x, double y, double* result) {
  return Calc("subtract", "x", "y", x, y, result);
}

bool Multiply(double x, double y, double* result) {
  return Calc("multiply", "x", "y", x, y, result);
}

bool Divide(double x, double y, double* result) {
  return Calc("divide", "numerator", "denominator", x, y, result);
}
```
See? It's not that complicated. Check folder ***demo/calculator_client*** for the full example.

## Server Usage

*NOTE: The server part is currently under development, not stable enough to be used in a real production.*

Suppose you want to provide a calculator web service just like the one from ParaSoft.

Firstly, create a class `CalculatorService` which is derived from `csoap::SoapService`, override the `Handle` method:
```cpp
#include "csoap/soap_service.h"

class CalculatorService : public csoap::SoapService {
public:
  CalculatorService();

  bool Handle(const csoap::SoapRequest& soap_request,
              csoap::SoapResponse* soap_response) override;
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
#include "calculator_service.h"

#include "boost/lexical_cast.hpp"

#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

CalculatorService::CalculatorService() {
}

bool CalculatorService::Handle(const csoap::SoapRequest& soap_request,
                               csoap::SoapResponse* soap_response) {
  try {
    if (soap_request.operation() == "add") {
      double x = boost::lexical_cast<double>(soap_request.GetParameter("x"));
      double y = boost::lexical_cast<double>(soap_request.GetParameter("y"));

      double result = x + y;

      soap_response->set_soapenv_ns(csoap::kSoapEnvNamespace);
      soap_response->set_service_ns({ "cal", "http://mycalculator/" });
      soap_response->set_operation(soap_request.operation());
      soap_response->set_result_name("Result");
      soap_response->set_result(boost::lexical_cast<std::string>(result));

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

  const char* host = "0.0.0.0";  // TODO

  try {
    csoap::HttpServer server(host, argv[1]);

    csoap::SoapServicePtr service(new CalculatorService);
    server.RegisterService(service);

    server.Run();

  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
```

## Limitations

- Only support `int`, `double`, `bool` and `string` parameters.
- Only support UTF-8 encoded content.
- One connection one call.
- Connection is in synchronous (or blocking) mode; timeout (default to 30s) is configurable.

## Dependencies

- Boost 1.66+.
- XML processing is based on PugiXml, which is already included in the source tree.
- Build system is CMake, but it should be quite easy to integrate into your own project.
