#ifndef CSOAP_SOAP_REQUEST_ENVELOPE_H_
#define CSOAP_SOAP_REQUEST_ENVELOPE_H_

#include <string>
#include <vector>

#include "csoap/common.h"

namespace csoap {

// SOAP request envelope.
// Used to compose the SOAP envelope XML which will be sent as the HTTP
// request body.
class SoapRequestEnvelope {
public:
  enum NSType {
    kSoapEnvelopeNS = 0,
    kServiceNS,
    kCountNS,
  };

public:
  explicit SoapRequestEnvelope(const std::string& operation);

  void SetNamespace(NSType ns_type, const Namespace& ns);

  void SetNamespace(NSType ns_type,
                    const std::string& name,
                    const std::string& url);

  void AddParameter(const std::string& key, const std::string& value);
  void AddParameter(const Parameter& parameter);

  void ToXmlString(std::string* xml_string);

private:
  Namespace namespaces_[kCountNS];

  std::string operation_;
  std::vector<Parameter> parameters_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_REQUEST_ENVELOPE_H_
