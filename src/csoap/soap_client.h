#ifndef CSOAP_SOAP_CLIENT_H_
#define CSOAP_SOAP_CLIENT_H_

#include <string>
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
  bool Call(const std::string& operation,
            const csoap::Parameter* parameters,
            size_t count,
            std::string* result);

protected:
  // Request URL.
  // Could be a complete URL (http://ws1.parasoft.com/glue/calculator)
  // or just the path component of it (/glue/calculator).
  std::string url_;

  std::string host_;
  std::string port_;  // Leave this empty to use default 80.

  // The namespace of your service.
  csoap::Namespace service_ns_;

  // Response result XML node name.
  // E.g., "Result".
  std::string result_name_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_CLIENT_H_
