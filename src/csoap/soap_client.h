#ifndef CSOAP_SOAP_CLIENT_H_
#define CSOAP_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "csoap/common.h"

namespace csoap {

// Base class for your SOAP client.
// Set URL, host, port, etc. in your sub-class before make the call.
//
class SoapClient {
public:
  virtual ~SoapClient() {
  }

protected:
  SoapClient() {
  }

  // A generic wrapper to make a call.
  // NOTE: The parameters should be movable.
  Error Call(const std::string& operation,
             std::vector<Parameter>&& parameters,
             std::string* result);

protected:
  Namespace soapenv_ns_;  // SOAP envelope namespace.
  Namespace service_ns_;  // Namespace for your web service.

  // Request URL.
  std::string url_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_CLIENT_H_
