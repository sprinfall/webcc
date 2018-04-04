#include "csoap/soap_client.h"

#include <cassert>

#include "csoap/http_client.h"
#include "csoap/http_request.h"
#include "csoap/http_response.h"
#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

namespace csoap {

Error SoapClient::Call(const std::string& operation,
                       std::vector<Parameter>&& parameters,
                       std::string* result) {
  assert(service_ns_.IsValid());
  assert(!url_.empty() && !host_.empty());
  assert(!result_name_.empty());

  if (!soapenv_ns_.IsValid()) {
    soapenv_ns_ = kSoapEnvNamespace;
  }

  SoapRequest soap_request;

  soap_request.set_soapenv_ns(soapenv_ns_);
  soap_request.set_service_ns(service_ns_);

  soap_request.set_operation(operation);

  for (Parameter& p : parameters) {
    soap_request.AddParameter(std::move(p));
  }

  std::string http_content;
  soap_request.ToXml(&http_content);

  HttpRequest http_request;

  http_request.set_method(kHttpPost);
  http_request.set_url(url_);
  http_request.SetContentType(kTextXmlUtf8);
  http_request.SetContentLength(http_content.size());
  http_request.SetHost(host_, port_);
  http_request.SetHeader(kSoapAction, operation);
  http_request.set_content(std::move(http_content));

  http_request.MakeStartLine();

  HttpResponse http_response;

  HttpClient http_client;
  Error error = http_client.SendRequest(http_request, &http_response);

  if (error != kNoError) {
    return error;
  }

  SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (!soap_response.FromXml(http_response.content())) {
    return kXmlError;
  }

  *result = soap_response.result();

  return kNoError;
}

}  // namespace csoap
