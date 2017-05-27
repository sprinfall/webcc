#ifndef CSOAP_RESPONSE_H_
#define CSOAP_RESPONSE_H_

#include <string>

namespace csoap {

// SOAP response.
// Used to parse the SOAP response XML which is returned as the HTTP response
// body.
class SoapResponse {
public:
  SoapResponse();
 
  bool Parse(const std::string& content,
             const std::string& message_name,
             const std::string& element_name,
             std::string* element_value);

  const std::string& soapenv_ns() const {
    return soapenv_ns_;
  }

private:
  // Soap envelope namespace in the response XML.
  std::string soapenv_ns_;
};

}  // namespace csoap

#endif  // CSOAP_RESPONSE_H_
