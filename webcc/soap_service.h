#ifndef WEBCC_SOAP_SERVICE_H_
#define WEBCC_SOAP_SERVICE_H_

#include <memory>

#include "webcc/globals.h"

namespace webcc {

class SoapRequest;
class SoapResponse;

// Base class for your SOAP service.
class SoapService {
public:
  virtual ~SoapService() = default;

  // Handle SOAP request, output the response.
  virtual bool Handle(const SoapRequest& soap_request,
                      SoapResponse* soap_response) = 0;

protected:
  http::Status http_status_ = http::Status::kOK;
};

typedef std::shared_ptr<SoapService> SoapServicePtr;

}  // namespace webcc

#endif  // WEBCC_SOAP_SERVICE_H_
