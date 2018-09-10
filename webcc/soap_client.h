#ifndef WEBCC_SOAP_CLIENT_H_
#define WEBCC_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "webcc/http_client.h"
#include "webcc/soap_globals.h"
#include "webcc/soap_parameter.h"

namespace webcc {

class SoapClient {
 public:
  // If |port| is empty, |host| will be checked to see if it contains port or
  // not (separated by ':').
  explicit SoapClient(const std::string& host, const std::string& port = "");

  ~SoapClient() = default;

  WEBCC_DELETE_COPY_ASSIGN(SoapClient);

  void SetTimeout(int seconds) {
    http_client_.SetTimeout(seconds);
  }

  void set_url(const std::string& url) { url_ = url; }

  void set_service_ns(const SoapNamespace& service_ns) {
    service_ns_ = service_ns;
  }

  void set_result_name(const std::string& result_name) {
    result_name_ = result_name;
  }

  void set_format_raw(bool format_raw) {
    format_raw_ = format_raw;
  }

  void set_indent_str(const std::string& indent_str) {
    indent_str_ = indent_str;
  }

  bool Request(const std::string& operation,
               std::vector<SoapParameter>&& parameters,
               std::string* result);

  bool timed_out() const {
    return http_client_.timed_out();
  }

  Error error() const { return error_; }

 private:
  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // Request URL.
  std::string url_;

  SoapNamespace soapenv_ns_;  // SOAP envelope namespace.
  SoapNamespace service_ns_;  // Namespace for your web service.

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;

  // Format request XML without any indentation or line breaks.
  bool format_raw_;

  // Indent string for request XML.
  // Applicable when |format_raw_| is false.
  std::string indent_str_;

  HttpClient http_client_;

  Error error_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_CLIENT_H_
