#ifndef CSOAP_RESPONSE_PARSER_H_
#define CSOAP_RESPONSE_PARSER_H_

#include <string>

namespace csoap {

class SoapResponseParser {
public:
  SoapResponseParser();

  // <soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/">
  //   <soapenv:Body>
  //     <ns:getPatientResponse xmlns:ns="http://service.csdm.carestream.com">
  //       <ns:return> ...
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

#endif  // CSOAP_RESPONSE_PARSER_H_
