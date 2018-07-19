#ifndef WEBCC_SOAP_CLIENT_H_
#define WEBCC_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "webcc/soap_message.h"
#include "webcc/soap_parameter.h"

namespace webcc {

// Base class for your SOAP client.
// Set URL, host, port, etc. in your sub-class before make the call.
class SoapClient {
 public:
  virtual ~SoapClient() = default;

  bool timed_out() const { return timed_out_; }

  void set_format_raw(bool format_raw) { format_raw_ = format_raw; }

  void set_indent_str(const std::string& indent_str) {
    indent_str_ = indent_str;
  }

 protected:
  SoapClient(const std::string& host, const std::string& port);

  // A generic wrapper to make a call.
  // NOTE: The parameters should be movable.
  Error Call(const std::string& operation,
             std::vector<SoapParameter>&& parameters,
             std::string* result);

  SoapNamespace soapenv_ns_;  // SOAP envelope namespace.
  SoapNamespace service_ns_;  // Namespace for your web service.

  // Request URL.
  std::string url_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;

  // Format request XML without any indentation or line breaks.
  bool format_raw_;

  // Indent string for request XML.
  // Applicable when |format_raw_| is false.
  std::string indent_str_;

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_;

  // If the error was caused by timeout or not.
  bool timed_out_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_CLIENT_H_
