#ifndef WEBCC_SOAP_CLIENT_H_
#define WEBCC_SOAP_CLIENT_H_

#include <string>
#include <vector>

#include "webcc/common.h"

namespace webcc {

// Base class for your SOAP client.
// Set URL, host, port, etc. in your sub-class before make the call.
//
class SoapClient {
public:
  virtual ~SoapClient() = default;

protected:
  SoapClient() = default;

  // A generic wrapper to make a call.
  // NOTE: The parameters should be movable.
  Error Call(const std::string& operation,
             std::vector<Parameter>&& parameters,
             std::string* result);

protected:
  // -1 means default timeout (normally 30s) will be used.
  int timeout_seconds_ = -1;

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

}  // namespace webcc

#endif  // WEBCC_SOAP_CLIENT_H_
