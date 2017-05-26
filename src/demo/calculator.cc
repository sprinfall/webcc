#include "demo/calculator.h"

#include <iostream>
#include "pugixml/pugixml.hpp"
#include "csoap/csoap.h"

namespace demo {

Calculator::Calculator() {
  Init();
}

bool Calculator::Add(float x, float y, float* result) {
  csoap::Parameter parameters[] = {
    { "x", x },
    { "y", y }
  };

  std::string result_str;
  if (!Call("add", parameters, 2, &result_str)) {
    return false;
  }

  try {
    *result = boost::lexical_cast<float>(result_str);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  return true;
}

void Calculator::Init() {
  soap_envelope_ns_ = { "soapenv", "http://schemas.xmlsoap.org/soap/envelope/" };
  service_ns_ = { "ser", "http://www.parasoft.com/wsdl/calculator/" };

  host_ = "ws1.parasoft.com";
  port_ = "";  // Use default: 80
}

bool Calculator::Call(const std::string& operation,
                      const csoap::Parameter* parameters,
                      size_t count,
                      std::string* result) {
  csoap::SoapRequestEnvelope req_envelope(operation);

  req_envelope.SetNamespace(csoap::SoapRequestEnvelope::kSoapEnvelopeNS, soap_envelope_ns_);
  req_envelope.SetNamespace(csoap::SoapRequestEnvelope::kServiceNS, service_ns_);

  for (size_t i = 0; i < count; ++i) {
    req_envelope.AddParameter(parameters[i]);
  }

  std::string request_body;
  req_envelope.ToXmlString(&request_body);

  csoap::HttpRequest http_request(csoap::kHttpV11);

  http_request.set_uri("http://ws1.parasoft.com/glue/calculator");
  http_request.set_content_type("text/xml; charset=utf-8");
  http_request.set_content_length(request_body.size());
  http_request.set_host(host_, port_);
  http_request.set_keep_alive(true);

  http_request.set_soap_action(operation);

  csoap::HttpClient http_client;

  if (!http_client.SendRequest(http_request, request_body)) {
    std::cerr << "Failed to send HTTP request." << std::endl;
    return false;
  }

  const csoap::HttpResponse& http_response = http_client.response();

  std::cout << http_response.status() << " " << http_response.reason() << std::endl;

  csoap::SoapResponseParser soap_response_parser;

  std::string rsp_message_name = operation + "Response";
  std::string rsp_element_name = "Result";

  soap_response_parser.Parse(http_response.content(),
                             rsp_message_name,
                             rsp_element_name,
                             result);

  //std::cout << "return:\n" << *result << std::endl;

  return true;
}

}  // namespace demo
