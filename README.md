# csoap

[中文版介绍](https://segmentfault.com/a/1190000009874151)

A lightweight C++ SOAP client & server library based on Boost.Asio.

NOTE: The server part is currently under development, not stable enough to be used in a real production. 

## Client Usage

Firstly, please install SoapUI if you don't have it. We need SoapUI to generate sample requests for each web service operation. The open source version is good enough.

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
  csoap::Parameter parameters[] = {
    { x_name, x },
    { y_name, y }
  };

  // Make the call.
  std::string result_str;
  if (!Call(operation, parameters, 2, &result_str)) {
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
TODO

## Limitations

- Only support `int`, `double`, `bool` and `string` parameters.
- Only support UTF-8 encoded content.
- One connection one call.
- Connection is in synchronous (or blocking) mode; timeout (default to 30s) is configurable.

## Dependencies

- Boost.Asio is used for both client and server.
- XML processing is based on PugiXml, which is already included in the source tree.
- Build system is CMake, but it should be quite easy to integrate into your own project.
