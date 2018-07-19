#ifndef WEBCC_SOAP_REQUEST_H_
#define WEBCC_SOAP_REQUEST_H_

#include <string>
#include <vector>

#include "webcc/soap_message.h"
#include "webcc/soap_parameter.h"

namespace webcc {

// SOAP request.
// Used to compose the SOAP request envelope XML which will be sent as the HTTP
// request body.
class SoapRequest : public SoapMessage {
 public:
  void AddParameter(const SoapParameter& parameter);

  void AddParameter(SoapParameter&& parameter);

  // Get parameter value by key.
  const std::string& GetParameter(const std::string& key) const;

 protected:
  void ToXmlBody(pugi::xml_node xbody) override;
  bool FromXmlBody(pugi::xml_node xbody) override;

 private:
  std::vector<SoapParameter> parameters_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_REQUEST_H_
