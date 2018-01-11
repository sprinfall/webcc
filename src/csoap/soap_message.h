#ifndef CSOAP_SOAP_MESSAGE_H_
#define CSOAP_SOAP_MESSAGE_H_

#include <string>
#include "pugixml/pugixml.hpp"

#include "csoap/common.h"

namespace csoap {

// Base class for SOAP request and response.
class SoapMessage {
public:
  // E.g., set as kSoapEnvNamespace.
  CLIENT_API void set_soapenv_ns(const Namespace& soapenv_ns) {
    soapenv_ns_ = soapenv_ns;
  }

  CLIENT_API void set_service_ns(const Namespace& service_ns) {
    service_ns_ = service_ns;
  }

  SERVER_API const std::string& operation() const {
    return operation_;
  }

  CLIENT_API void set_operation(const std::string& operation) {
    operation_ = operation;
  }

  // Convert to SOAP request XML.
  CLIENT_API void ToXml(std::string* xml_string);

  // Parse from SOAP request XML.
  SERVER_API bool FromXml(const std::string& xml_string);

protected:
  SoapMessage() {
  }

  // Convert to SOAP body XML.
  virtual void ToXmlBody(pugi::xml_node xbody) = 0;

  // Parse from SOAP body XML.
  virtual bool FromXmlBody(pugi::xml_node xbody) = 0;

protected:
  Namespace soapenv_ns_;  // SOAP envelope namespace.
  Namespace service_ns_;  // Namespace for your web service.

  std::string operation_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_MESSAGE_H_
