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

bool SoapClient::Call(const std::string& operation,
                      const csoap::Parameter* parameters,
                      size_t count,
                      std::string* result) {
  assert(!url_.empty() &&
         !host_.empty() &&
         !result_name_.empty() &&
         service_ns_.IsValid());

  csoap::SoapRequest soap_request;

  soap_request.set_soapenv_ns(kSoapEnvNamespace);
  soap_request.set_service_ns(service_ns_);

  soap_request.set_operation(operation);

  for (size_t i = 0; i < count; ++i) {
    soap_request.AddParameter(parameters[i]);
  }

  std::string http_request_body;
  soap_request.ToXml(&http_request_body);

  csoap::HttpRequest http_request;

  http_request.set_version(csoap::kHttpV11);
  http_request.set_url(url_);
  http_request.set_content_length(http_request_body.size());
  http_request.set_content(http_request_body);  // TODO: move
  http_request.set_host(host_, port_);
  http_request.set_soap_action(operation);

  csoap::HttpResponse http_response;

  csoap::HttpClient http_client;
  csoap::ErrorCode ec = http_client.SendRequest(http_request, &http_response);
  if (ec != csoap::kNoError) {
    std::cerr << csoap::GetErrorMessage(ec) << std::endl;

    if (ec == csoap::kHttpStatusError) {
      //std::cerr << "\t"
      //  << http_response.status() << ", "
      //  << http_response.reason() << std::endl;
    }

    return false;
  }

  csoap::SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (soap_response.FromXml(http_response.content())) {
    *result = soap_response.result();
    return true;
  }

  return false;
}

}  // namespace csoap
