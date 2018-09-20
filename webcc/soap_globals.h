#ifndef WEBCC_SOAP_GLOBALS_H_
#define WEBCC_SOAP_GLOBALS_H_

#include <iosfwd>
#include <string>

namespace webcc {

// -----------------------------------------------------------------------------
// Constants

extern const std::string kSoapAction;

extern const std::string kTextXmlUtf8;
extern const std::string kAppSoapXmlUtf8;

// -----------------------------------------------------------------------------

enum SoapVersion {
  kSoapV11,
  kSoapV12,
};

// XML namespace name/url pair.
// E.g., { "soap", "http://www.w3.org/2003/05/soap-envelope" }
struct SoapNamespace {
  std::string name;
  std::string url;

  bool IsValid() const {
    return !name.empty() && !url.empty();
  }
};

// Default namespaces for SOAP Envelope.
extern const SoapNamespace kSoapEnvNamespaceV11;
extern const SoapNamespace kSoapEnvNamespaceV12;

struct SoapFault {
  std::string faultcode;
  std::string faultstring;
  std::string detail;
};

std::ostream& operator<<(std::ostream& os, const SoapFault& fault);

}  // namespace webcc

#endif  // WEBCC_SOAP_GLOBALS_H_
