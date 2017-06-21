# csoap
A lightweight C++ SOAP client library.

[中文版介绍](https://segmentfault.com/a/1190000009874151)

## Usage

Firstly, please install SoapUI if you don't have it. We need SoapUI to generate sample requests for each web service operation. The open source version is good enough.

Take the calculator web service provided by ParaSoft as an example. Download the WSDL from http://ws1.parasoft.com/glue/calculator.wsdl, create a SOAP project within SoapUI (remember to check "Create sample requests for all operations?"), you will see the sample request for "add" operation as the following:
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

In order to call the "add" operation, we have to send a HTTP request with the above SOAP envelope as the content. Let's see how to do this with csoap.

```cpp
bool Add() {
  // Create a SOAP request.
  csoap::SoapRequest soap_request("add");

  // Set the service namespace.
  csoap::Namespace service_ns = { "cal", "http://www.parasoft.com/wsdl/calculator/" };
  soap_request.set_service_ns(service_ns);

  // Add parameters for the operation.
  soap_request.AddParameter("x", "1.0");
  soap_request.AddParameter("y", "2.0");

  // Generate the XML string for the SOAP envelope.
  std::string http_request_body;
  soap_request.ToXmlString(&http_request_body);

  // Create a HTTP request.
  csoap::HttpRequest http_request(csoap::kHttpV11);

  // Set the HTTP request headers.
  http_request.set_url("/glue/calculator");
  http_request.set_content_length(http_request_body.size());
  http_request.set_host("ws1.parasoft.com", "");
  http_request.set_soap_action("add");

  // Send the HTTP request and get the HTTP response.

  csoap::HttpResponse http_response;

  csoap::HttpClient http_client;
  csoap::ErrorCode ec = http_client.SendRequest(http_request,
                                                http_request_body,
                                                &http_response);

  // Error handling.
  if (ec != csoap::kNoError) {
    std::cerr << csoap::GetErrorMessage(ec) << std::endl;
    return false;
  }

  // Parse the SOAP response.
  std::string result;
  csoap::SoapResponse soap_response;
  if (!soap_response.Parse(http_response.content(), "addResponse", "Result", &result)) {
    return false;
  }

  std::cout << result << std::endl;

  return true;
}
```

It's not that complicated. But you can't code like this for each operation. You have to do some encapsulation, make a general function, add a base class, and so on. Check the "demo" folder for the detailed example.

## Limitations

- Only support HTTP 1.1.
- Only support `int`, `double`, `bool` and `string` parameters.
- Only support UTF-8 encoded content.

## Dependencies

- The TCP socket client is based on Boost.Asio.
- The XML parsing is based on TinyXml or PugiXml, a macro `CSOAP_USE_TINYXML` controls which one to use.
- Build system is CMake, but it should be easy to integrate into your project.
