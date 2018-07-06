#ifndef WEBCC_SOAP_MESSAGE_H_
#define WEBCC_SOAP_MESSAGE_H_

#include <string>

#include "pugixml/pugixml.hpp"

#include "webcc/globals.h"

namespace webcc {

// XML namespace name/url pair.
// E.g., { "soap", "http://schemas.xmlsoap.org/soap/envelope/" }
class SoapNamespace {
 public:
  std::string name;
  std::string url;

  bool IsValid() const {
    return !name.empty() && !url.empty();
  }
};

// CSoap's default namespace for SOAP Envelope.
extern const SoapNamespace kSoapEnvNamespace;

// Base class for SOAP request and response.
class SoapMessage {
 public:
  virtual ~SoapMessage() {}

  // E.g., set as kSoapEnvNamespace.
  void set_soapenv_ns(const SoapNamespace& soapenv_ns) {
    soapenv_ns_ = soapenv_ns;
  }

  void set_service_ns(const SoapNamespace& service_ns) {
    service_ns_ = service_ns;
  }

  const std::string& operation() const {
    return operation_;
  }

  void set_operation(const std::string& operation) {
    operation_ = operation;
  }

  // Convert to SOAP request XML.
  void ToXml(std::string* xml_string);

  // Parse from SOAP request XML.
  bool FromXml(const std::string& xml_string);

 protected:
  // Convert to SOAP body XML.
  virtual void ToXmlBody(pugi::xml_node xbody) = 0;

  // Parse from SOAP body XML.
  virtual bool FromXmlBody(pugi::xml_node xbody) = 0;

  SoapNamespace soapenv_ns_;  // SOAP envelope namespace.
  SoapNamespace service_ns_;  // Namespace for your web service.

  std::string operation_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_MESSAGE_H_
