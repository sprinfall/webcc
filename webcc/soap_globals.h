#ifndef WEBCC_SOAP_GLOBALS_H_
#define WEBCC_SOAP_GLOBALS_H_

#include <string>

namespace webcc {

// -----------------------------------------------------------------------------
// Constants

extern const std::string kSoapAction;

extern const std::string kTextXmlUtf8;

// -----------------------------------------------------------------------------

// XML namespace name/url pair.
// E.g., { "soap", "http://schemas.xmlsoap.org/soap/envelope/" }
struct SoapNamespace {
  std::string name;
  std::string url;

  bool IsValid() const {
    return !name.empty() && !url.empty();
  }
};

// CSoap's default namespace for SOAP Envelope.
extern const SoapNamespace kSoapEnvNamespace;

}  // namespace webcc

#endif  // WEBCC_SOAP_GLOBALS_H_
