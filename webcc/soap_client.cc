#include "webcc/soap_client.h"

#include <cassert>
#include <utility>  // for move()

#include "webcc/http_client.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

SoapClient::SoapClient(const std::string& host, const std::string& port)
    : host_(host), port_(port),
      soapenv_ns_(kSoapEnvNamespace),
      format_raw_(true), timeout_seconds_(0), timed_out_(false),
      error_(kNoError) {
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

  SoapRequest soap_request;

  soap_request.set_soapenv_ns(soapenv_ns_);
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
  http_request.SetContentType(kTextXmlUtf8);
  http_request.SetHost(host_, port_);
  http_request.SetHeader(kSoapAction, operation);
  http_request.UpdateStartLine();

  HttpClient http_client;

  if (timeout_seconds_ > 0) {
    http_client.set_timeout_seconds(timeout_seconds_);
  }

  if (!http_client.Request(http_request)) {
    error_ = http_client.error();
    timed_out_ = http_client.timed_out();
    return false;
  }

  SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (!soap_response.FromXml(http_client.response()->content())) {
    error_ = kXmlError;
    return false;
  }

  *result = soap_response.result_moved();

  return true;
}

}  // namespace webcc
