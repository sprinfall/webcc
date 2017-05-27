#ifndef CSOAP_SOAP_REQUEST_H_
#define CSOAP_SOAP_REQUEST_H_

#include <string>
#include <vector>

#include "csoap/common.h"

namespace csoap {

// SOAP request.
// Used to compose the SOAP request envelope XML which will be sent as the HTTP
// request body.
class SoapRequest {
public:
  explicit SoapRequest(const std::string& operation);

  // Set the name of SOAP envelope namespace if you don't like the default
  // name "soapenv".
  void set_soapenv_ns_name(const std::string& name) {
    soapenv_ns_.name = name;
  }

  void set_service_ns(const Namespace& ns) {
    service_ns_ = ns;
  }

  void AddParameter(const std::string& key, const std::string& value);
  void AddParameter(const Parameter& parameter);

  void ToXmlString(std::string* xml_string);

private:
  // SOAP envelope namespace.
  // The URL is always "http://schemas.xmlsoap.org/soap/envelope/".
  // The name is "soapenv" by default.
  Namespace soapenv_ns_;

  // Namespace for your web service.
  Namespace service_ns_;

  std::string operation_;
  std::vector<Parameter> parameters_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_REQUEST_H_
