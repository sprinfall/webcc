# SOAP Server Tutorial

Suppose you want to provide a calculator web service just like the one from [ParaSoft](http://ws1.parasoft.com/glue/calculator.wsdl).

Firstly, create a class `CalcService` which is derived from `webcc::SoapService`, override the `Handle` method:
```cpp
// calc_service.h

#include "webcc/soap_service.h"

class CalcService : public webcc::SoapService {
public:
  bool Handle(const webcc::SoapRequest& soap_request,
              webcc::SoapResponse* soap_response) override;
};
```

The `Handle` method has two parameters, one for request (input), one for response (output).

The implementation is quite straightforward:

- Get operation (e.g., add) from request;
- Get parameters (e.g., x and y) from request;
- Calculate the result.
- Set namespaces, result name and so on to response.
- Set result to response.

```cpp
// calc_service.cpp

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
      soap_response->set_service_ns({ "cal", "http://www.example.com/calculator/" });
      soap_response->set_operation(soap_request.operation());
      soap_response->set_result_name("Result");
      soap_response->set_result(std::to_string(result));

      return true;
    }

    // Other operations ...

  } catch (boost::bad_lexical_cast&) {
    // ...
  }

  return false;
}
```

Next step, create a `SoapServer` and register `CalcService` to it with a URL.

```cpp
int main(int argc, char* argv[]) {
  // Check argc and argv ...

  unsigned short port = std::atoi(argv[1]);

  // Number of worker threads.
  std::size_t workers = 2;

  try {
    webcc::SoapServer server(port, workers);

    server.RegisterService(std::make_shared<CalcService>(), "/calculator");

    server.Run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
```

The server is created with a **port number** which will be listened on to **asynchnously** accept client connections. The connections will be firstly put into a **queue** and then processed by the **worker threads**. The number of worker threads is determined by the `workers` parameter.

When register service, the URL is what the clients will put in the HTTP request to access your service:
```
POST /calculator HTTP/1.1
```

Registering multiple services to a server is allowed, but the URL must be unique for each service.

To invoke the `add` operation of the calculator service, an example of the client HTTP request would be:
```
POST /calculator HTTP/1.1
Content-Type: text/xml; charset=utf-8
Content-Length: 263
Host: localhost:8080
SOAPAction: add

<?xml version="1.0"?>
<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
        <soap:Body>
                <ser:add xmlns:ser="http://www.example.com/calculator/">
                        <ser:x>1.000000</ser:x>
                        <ser:y>2.000000</ser:y>
                </ser:add>
        </soap:Body>
</soap:Envelope>
```

And the HTTP response is:
```
HTTP/1.1 200 OK
Content-Type: text/xml; charset=utf-8
Content-Length: 262

<?xml version="1.0"?>
<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
        <soap:Body>
                <cal:addResponse xmlns:cal="http://www.example.com/calculator/">
                        <cal:Result>3.000000</cal:Result>
                </cal:addResponse>
        </soap:Body>
</soap:Envelope>
```

See [example/soap_calc_server](https://github.com/sprinfall/webcc/tree/master/example/soap_calc_server) for the full example.
