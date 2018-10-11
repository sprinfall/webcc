#include "webcc/soap_client.h"

#include <cassert>
#include <utility>  // for move()

#include "boost/algorithm/string.hpp"

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
                         SoapResponse::Parser parser) {
  assert(service_ns_.IsValid());
  assert(!url_.empty() && !host_.empty());

  error_ = kNoError;
  fault_.reset();

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

  http_request.set_host(host_, port_);
  http_request.SetHeader(kSoapAction, operation);
  http_request.Make();

  if (!http_client_.Request(http_request)) {
    error_ = http_client_.error();
    return false;
  }

  SoapResponse soap_response;
  soap_response.set_operation(operation);
  soap_response.set_parser(parser);

  if (!soap_response.FromXml(http_client_.response()->content())) {
    if (soap_response.fault()) {
      fault_ = soap_response.fault();
      error_ = kServerError;
    } else {
      error_ = kXmlError;
    }

    return false;
  }

  return true;
}

bool SoapClient::Request(const std::string& operation,
                         std::vector<SoapParameter>&& parameters,
                         const std::string& result_name,
                         std::string* result) {
  auto parser = [result, &result_name](pugi::xml_node xnode) {
    if (boost::iequals(soap_xml::GetNameNoPrefix(xnode), result_name)) {
      soap_xml::GetText(xnode, result);
    }
    return false;  // Stop next call.
  };

  return Request(operation, std::move(parameters), parser);
}

}  // namespace webcc
