#include "webcc/soap_client.h"

#include <cassert>
#include <utility>  // for move()

#include "boost/algorithm/string.hpp"

#include "webcc/soap_request.h"
#include "webcc/utility.h"

namespace webcc {

SoapClient::SoapClient(const std::string& url, SoapVersion soap_version)
    : url_(url),
      soap_version_(soap_version),
      http_client_(true),
      format_raw_(true),
      error_(kNoError) {
}

bool SoapClient::Request(const std::string& operation,
                         std::vector<SoapParameter>&& parameters,
                         SoapResponse::Parser parser,
                         std::size_t buffer_size) {
  assert(service_ns_.IsValid());
  assert(!url_.empty());

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

  auto http_request = std::make_shared<HttpRequest>(http::kPost, url_);

  http_request->SetContent(std::move(http_content), true);

  if (soap_version_ == kSoapV11) {
    http_request->SetContentType(http::media_types::kTextXml,
                                 http::charsets::kUtf8);
  } else {
    http_request->SetContentType(http::media_types::kApplicationSoapXml,
                                 http::charsets::kUtf8);
  }

  http_request->SetHeader(kSoapAction, operation);

  http_request->set_buffer_size(buffer_size);

  http_request->Prepare();

  if (!http_client_.Request(http_request, true)) {
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
                         std::size_t buffer_size,
                         std::string* result) {
  auto parser = [result, &result_name](pugi::xml_node xnode) {
    if (boost::iequals(soap_xml::GetNameNoPrefix(xnode), result_name)) {
      soap_xml::GetText(xnode, result);
    }
    return false;  // Stop next call.
  };

  return Request(operation, std::move(parameters), parser, buffer_size);
}

}  // namespace webcc
