#ifndef CSOAP_SOAP_RESPONSE_H_
#define CSOAP_SOAP_RESPONSE_H_

#include "csoap/soap_message.h"

namespace csoap {

// SOAP response.
class SoapResponse : public SoapMessage {
public:
  // Could be "Price" for an operation/method like "GetXyzPrice".
  // Really depend on the service.
  // Most services use a general name "Result".
  CLIENT_API void set_result_name(const std::string& result_name) {
    result_name_ = result_name;
  }

  CLIENT_API const std::string& result() const {
    return result_;
  }

  SERVER_API void set_result(const std::string& result) {
    result_ = result;
  }

protected:
  void ToXmlBody(pugi::xml_node xbody) override;

  bool FromXmlBody(pugi::xml_node xbody) override;

private:
  // TODO: Support multiple results.

  // Result XML node name.
  // Used to parse the response XML from client side.
  std::string result_name_;

  // Result value.
  std::string result_;
};

}  // namespace csoap

#endif  // CSOAP_SOAP_RESPONSE_H_
