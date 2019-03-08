#ifndef WEBCC_SOAP_CLIENT_H_
#define WEBCC_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "webcc/http_client.h"
#include "webcc/soap_globals.h"
#include "webcc/soap_parameter.h"
#include "webcc/soap_response.h"

namespace webcc {

class SoapClient {
public:
  // If |port| is empty, |host| will be checked to see if it contains port or
  // not (separated by ':').
  explicit SoapClient(const std::string& host, const std::string& port = "",
                      SoapVersion soap_version = kSoapV12,
                      std::size_t buffer_size = 0);

  ~SoapClient() = default;

  SoapClient(const SoapClient&) = delete;
  SoapClient& operator=(const SoapClient&) = delete;

  void SetTimeout(int seconds) {
    http_client_.SetTimeout(seconds);
  }

  void set_url(const std::string& url) { url_ = url; }

  void set_service_ns(const SoapNamespace& service_ns) {
    service_ns_ = service_ns;
  }

  void set_format_raw(bool format_raw) {
    format_raw_ = format_raw;
  }

  void set_indent_str(const std::string& indent_str) {
    indent_str_ = indent_str;
  }

  bool Request(const std::string& operation,
               std::vector<SoapParameter>&& parameters,
               SoapResponse::Parser parser,
               std::size_t buffer_size = 0);

  // Shortcut for responses with single result node.
  // The name of the single result node is specified by |result_name|.
  // The text of the result node will be set to |result|.
  bool Request(const std::string& operation,
               std::vector<SoapParameter>&& parameters,
               const std::string& result_name,
               std::size_t buffer_size,  // Pass 0 for using default size.
               std::string* result);

  // HTTP status code (200, 500, etc.) in the response.
  int http_status() const {
    assert(http_client_.response());
    return http_client_.response()->status();
  }

  bool timed_out() const {
    return http_client_.timed_out();
  }

  Error error() const { return error_; }

  std::shared_ptr<SoapFault> fault() const { return fault_; }

private:
  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  SoapVersion soap_version_;

  HttpClient http_client_;

  // Request URL.
  std::string url_;

  // Namespace for your web service.
  SoapNamespace service_ns_;

  // Format request XML without any indentation or line breaks.
  bool format_raw_;

  // Indent string for request XML.
  // Applicable when |format_raw_| is false.
  std::string indent_str_;

  Error error_;

  std::shared_ptr<SoapFault> fault_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_CLIENT_H_
