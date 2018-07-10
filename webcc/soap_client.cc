#include "webcc/soap_client.h"

#include <cassert>
#include <utility>  // for move()

#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

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
  http_request.SetContent(std::move(http_content));
  http_request.SetHost(host_, port_);
  http_request.SetHeader(kSoapAction, operation);
  http_request.UpdateStartLine();

  HttpResponse http_response;

  HttpClient http_client;

  if (timeout_seconds_ > 0) {
    http_client.set_timeout_seconds(timeout_seconds_);
  }

  if (!http_client.Request(http_request)) {
    timed_out_ = http_client.timed_out();
    return http_client.error();
  }

  SoapResponse soap_response;
  soap_response.set_result_name(result_name_);

  if (!soap_response.FromXml(http_client.response()->content())) {
    return kXmlError;
  }

  *result = soap_response.result_moved();

  return kNoError;
}

}  // namespace webcc
