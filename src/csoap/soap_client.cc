#include "csoap/soap_client.h"

#include <cassert>
#include <iostream>

#include "boost/lexical_cast.hpp"

#include "csoap/http_client.h"
#include "csoap/http_request.h"
#include "csoap/http_response.h"
#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

namespace csoap {

Error SoapClient::Call(const std::string& operation,
                       const Parameter* parameters,
                       std::size_t count,
                       std::string* result) {
  assert(!url_.empty() &&
         !host_.empty() &&
         !result_name_.empty() &&
         service_ns_.IsValid());

  SoapRequest soap_request;

  soap_request.set_soapenv_ns(kSoapEnvNamespace);  // TODO: Configurable
  soap_request.set_service_ns(service_ns_);

  soap_request.set_operation(operation);

  for (std::size_t i = 0; i < count; ++i) {
    soap_request.AddParameter(parameters[i]);
  }

  std::string http_request_body;
  soap_request.ToXml(&http_request_body);

  HttpRequest http_request;

  http_request.set_version(kHttpV11);  // TODO
  http_request.set_url(url_);
  http_request.set_content_length(http_request_body.size());
  http_request.set_content(http_request_body);  // TODO: move
  http_request.set_host(host_, port_);
  http_request.set_soap_action(operation);

  HttpResponse http_response;

  HttpClient http_client;
  Error error = http_client.SendRequest(http_request, &http_response);

  if (error != kNoError) {
    return error;
  }

  SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (!soap_response.FromXml(http_response.content())) {
    return kXmlError;  // TODO: Some SOAP error?
  }

  *result = soap_response.result();

  return kNoError;
}

}  // namespace csoap
