#ifndef CSOAP_SOAP_SERVICE_H_
#define CSOAP_SOAP_SERVICE_H_

#include <memory>

namespace csoap {

class SoapRequest;
class SoapResponse;

// Base class for your SOAP service.
class SoapService {
public:
  virtual ~SoapService() {
  }

  // Handle SOAP request, output the response.
  virtual bool Handle(const SoapRequest& soap_request,
                      SoapResponse* soap_response) = 0;
};

typedef std::shared_ptr<SoapService> SoapServicePtr;

}  // namespace csoap

#endif  // CSOAP_SOAP_SERVICE_H_
