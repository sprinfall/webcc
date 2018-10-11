#include "webcc/soap_async_client.h"

#include <cassert>
#include <utility>  // for move()

#include "webcc/http_async_client.h"
#include "webcc/soap_globals.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"

namespace webcc {

SoapAsyncClient::SoapAsyncClient(boost::asio::io_context& io_context,
                                 const std::string& host,
                                 const std::string& port,
                                 SoapVersion soap_version)
    : io_context_(io_context),
      host_(host), port_(port),
      soap_version_(soap_version),
      format_raw_(true), timeout_seconds_(0) {
  if (port_.empty()) {
    std::size_t i = host_.find_last_of(':');
    if (i != std::string::npos) {
      port_ = host_.substr(i + 1);
      host_ = host_.substr(0, i);
    }
  }
}

void SoapAsyncClient::Request(const std::string& operation,
                              std::vector<SoapParameter>&& parameters,
                              SoapResponseHandler soap_response_handler) {
  assert(service_ns_.IsValid());
  assert(!url_.empty() && !host_.empty());
  assert(!result_name_.empty());

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

  HttpRequestPtr http_request;

  http_request->set_method(kHttpPost);
  http_request->set_url(url_);
  http_request->SetContent(std::move(http_content), true);

  if (soap_version_ == kSoapV11) {
    http_request->SetContentType(kTextXmlUtf8);
  } else {
    http_request->SetContentType(kAppSoapXmlUtf8);
  }

  http_request->set_host(host_, port_);
  http_request->SetHeader(kSoapAction, operation);
  http_request->Make();

  HttpAsyncClientPtr http_client(new HttpAsyncClient(io_context_));

  if (timeout_seconds_ > 0) {
    http_client->set_timeout_seconds(timeout_seconds_);
  }

  auto http_response_handler = std::bind(&SoapAsyncClient::ResponseHandler,
                                         this, soap_response_handler,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3);

  http_client->Request(http_request, http_response_handler);
}

void SoapAsyncClient::ResponseHandler(SoapResponseHandler soap_response_handler,
                                      HttpResponsePtr http_response,
                                      Error error, bool timed_out) {
  if (error != kNoError) {
    soap_response_handler("", error, timed_out);
  } else {
    SoapResponse soap_response;
    //soap_response.set_result_name(result_name_);

    if (!soap_response.FromXml(http_response->content())) {
      soap_response_handler("", kXmlError, false);
    } else {
      //soap_response_handler(soap_response.result_moved(), kNoError, false);
    }
  }
}

}  // namespace webcc
