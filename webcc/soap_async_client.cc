#include "webcc/soap_async_client.h"

#include <cassert>
#include <utility>  // for move()

#include "webcc/soap_globals.h"
#include "webcc/soap_request.h"
#include "webcc/soap_response.h"
#include "webcc/utility.h"

namespace webcc {

SoapAsyncClient::SoapAsyncClient(boost::asio::io_context& io_context,
                                 const std::string& host,
                                 const std::string& port,
                                 SoapVersion soap_version,
                                 std::size_t buffer_size)
    : io_context_(io_context),
      host_(host), port_(port),
      soap_version_(soap_version),
      buffer_size_(buffer_size),
      format_raw_(true), timeout_seconds_(0) {
  AdjustHostPort(host_, port_);
}

void SoapAsyncClient::Request(const std::string& operation,
                              std::vector<SoapParameter>&& parameters,
                              SoapResponseCallback soap_response_callback) {
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

  HttpRequestPtr http_request(new HttpRequest(kHttpPost, url_, host_, port_));

  http_request->SetContent(std::move(http_content), true);

  if (soap_version_ == kSoapV11) {
    http_request->SetContentType(http::media_types::kTextXml,
                                 http::charsets::kUtf8);
  } else {
    http_request->SetContentType(http::media_types::kApplicationSoapXml,
                                 http::charsets::kUtf8);
  }

  http_request->SetHeader(kSoapAction, operation);
  http_request->Make();

  HttpAsyncClientPtr http_async_client{
    new HttpAsyncClient(io_context_, buffer_size_)
  };

  if (timeout_seconds_ > 0) {
    http_async_client->SetTimeout(timeout_seconds_);
  }

  auto http_response_callback = std::bind(&SoapAsyncClient::OnHttpResponse,
                                          this, soap_response_callback,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3);

  http_async_client->Request(http_request, http_response_callback);
}

void SoapAsyncClient::OnHttpResponse(SoapResponseCallback soap_response_callback,
                                     HttpResponsePtr http_response,
                                     Error error, bool timed_out) {
  if (error != kNoError) {
    soap_response_callback("", error, timed_out);
  } else {
    SoapResponse soap_response;
    // TODO
    //soap_response.set_result_name(result_name_);

    if (!soap_response.FromXml(http_response->content())) {
      soap_response_callback("", kXmlError, false);
    } else {
      // TODO
      //soap_response_callback(soap_response.result_moved(), kNoError, false);
    }
  }
}

}  // namespace webcc
