#ifndef CSOAP_SOAP_SERVICE_H_
#define CSOAP_SOAP_SERVICE_H_

#include <string>
#include <memory>

namespace csoap {

class SoapRequest;
class SoapResponse;

// Base class for your SOAP service.
class SoapService {
public:
  SoapService() {
  }

  virtual ~SoapService() {
  }

  // Handle SOAP request, output the response.
  virtual bool Handle(const SoapRequest& request,
                      SoapResponse* soap_response) = 0;

protected:
  // URL used to match the request.
  // E.g., "/", "/SomeService", etc.
  std::string url_;
};

typedef std::shared_ptr<SoapService> SoapServicePtr;

}  // namespace csoap

#endif  // CSOAP_SOAP_SERVICE_H_
