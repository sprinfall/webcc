#include "demo/calculator.h"

#include <iostream>
#include "pugixml/pugixml.hpp"
#include "csoap/csoap.h"

namespace demo {

Calculator::Calculator() {
  Init();
}

bool Calculator::Add(float x, float y, float* result) {
  return Calc("add", "x", "y", x, y, result);
}

bool Calculator::Subtract(float x, float y, float* result) {
  return Calc("subtract", "x", "y", x, y, result);
}

bool Calculator::Multiply(float x, float y, float* result) {
  return Calc("multiply", "x", "y", x, y, result);
}

bool Calculator::Divide(float x, float y, float* result) {
  return Calc("divide", "numerator", "denominator", x, y, result);
}

void Calculator::Init() {
  url_ = "/glue/calculator";

  host_ = "ws1.parasoft.com";
  port_ = "";  // Use default: 80

  service_ns_ = { "ser", "http://www.parasoft.com/wsdl/calculator/" };
}

bool Calculator::Calc(const std::string& operation,
                      const std::string& x_name,
                      const std::string& y_name,
                      float x,
                      float y,
                      float* result) {
  csoap::Parameter parameters[] = {
    { x_name, x },
    { y_name, y }
  };

  std::string result_str;
  if (!Call(operation, parameters, 2, &result_str)) {
    return false;
  }

  try {
    *result = boost::lexical_cast<float>(result_str);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  return true;
}

bool Calculator::Call(const std::string& operation,
                      const csoap::Parameter* parameters,
                      size_t count,
                      std::string* result) {
  csoap::SoapRequest soap_request(operation);

  soap_request.set_service_ns(service_ns_);

  for (size_t i = 0; i < count; ++i) {
    soap_request.AddParameter(parameters[i]);
  }

  std::string http_request_body;
  soap_request.ToXmlString(&http_request_body);

  csoap::HttpRequest http_request(csoap::kHttpV11);

  http_request.set_url(url_);
  http_request.set_content_length(http_request_body.size());
  http_request.set_host(host_, port_);
  http_request.set_soap_action(operation);

  csoap::HttpResponse http_response;

  csoap::HttpClient http_client;
  csoap::ErrorCode ec = http_client.SendRequest(http_request,
                                                http_request_body,
                                                &http_response);
  if (ec != csoap::kNoError) {
    std::cerr << csoap::GetErrorMessage(ec) << std::endl;

    if (ec == csoap::kHttpStatusError) {
      std::cerr << "\t"
                << http_response.status() << ", "
                << http_response.reason() << std::endl;
    }

    return false;
  }

  csoap::SoapResponse soap_response;

  std::string rsp_message_name = operation + "Response";
  std::string rsp_element_name = "Result";

  return soap_response.Parse(http_response.content(),
                             rsp_message_name,
                             rsp_element_name,
                             result);
}

}  // namespace demo
