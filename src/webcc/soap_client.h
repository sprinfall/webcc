#ifndef WEBCC_SOAP_CLIENT_H_
#define WEBCC_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "webcc/globals.h"
#include "webcc/soap_message.h"

namespace webcc {

// Base class for your SOAP client.
// Set URL, host, port, etc. in your sub-class before make the call.
class SoapClient {
 public:
  virtual ~SoapClient() = default;

  bool timed_out() const { return timed_out_; }

 protected:
  SoapClient() : timeout_seconds_(0), timed_out_(false) {
  }

  // A generic wrapper to make a call.
  // NOTE: The parameters should be movable.
  Error Call(const std::string& operation,
             std::vector<Parameter>&& parameters,
             std::string* result);

  // Timeout in seconds; only effective when > 0.
  int timeout_seconds_;

  // If the error was caused by timeout or not.
  bool timed_out_;

  SoapNamespace soapenv_ns_;  // SOAP envelope namespace.
  SoapNamespace service_ns_;  // Namespace for your web service.

  // Request URL.
  std::string url_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_CLIENT_H_
