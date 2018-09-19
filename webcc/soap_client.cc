#include "webcc/soap_client.h"

#include <cassert>
#include <utility>  // for move()

#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

SoapClient::SoapClient(const std::string& host, const std::string& port,
                       SoapVersion soap_version)
    : host_(host), port_(port),
      soap_version_(soap_version),
      format_raw_(true), error_(kNoError) {
  if (port_.empty()) {
    std::size_t i = host_.find_last_of(':');
    if (i != std::string::npos) {
      port_ = host_.substr(i + 1);
      host_ = host_.substr(0, i);
    }
  }
}

bool SoapClient::Request(const std::string& operation,
                         std::vector<SoapParameter>&& parameters,
                         std::string* result) {
  assert(service_ns_.IsValid());
  assert(!url_.empty() && !host_.empty());
  assert(!result_name_.empty());

  error_ = kNoError;

  SoapRequest soap_request;

  // Set SOAP envelope namespace according to SOAP version.
  if (soap_version_ == kSoapV11) {
    soap_request.set_soapenv_ns(kSoapEnvNamespaceV11);
  } else {
    soap_request.set_soapenv_ns(kSoapEnvNamespaceV12);
  }

  soap_request.set_service_ns(service_ns_);

  soap_request.set_operation(operation);

  for (SoapParameter& p : parameters) {
    soap_request.AddParameter(std::move(p));
  }

  std::string http_content;
  soap_request.ToXml(format_raw_, indent_str_, &http_content);

  HttpRequest http_request;

  http_request.set_method(kHttpPost);
  http_request.set_url(url_);
  http_request.SetContent(std::move(http_content), true);

  if (soap_version_ == kSoapV11) {
    http_request.SetContentType(kTextXmlUtf8);
  } else {
    http_request.SetContentType(kAppSoapXmlUtf8);
  }

  http_request.SetHost(host_, port_);
  http_request.SetHeader(kSoapAction, operation);
  http_request.UpdateStartLine();

  if (!http_client_.Request(http_request)) {
    error_ = http_client_.error();
    return false;
  }

  SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (!soap_response.FromXml(http_client_.response()->content())) {
    error_ = kXmlError;
    return false;
  }

  *result = soap_response.result_moved();

  return true;
}

}  // namespace webcc
