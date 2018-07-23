#ifndef CALC_SERVICE_H_
#define CALC_SERVICE_H_

#include "webcc/soap_service.h"

class CalcService : public webcc::SoapService {
 public:
  CalcService() = default;
  ~CalcService() override = default;

  bool Handle(const webcc::SoapRequest& soap_request,
              webcc::SoapResponse* soap_response) override;
};

#endif  // CALC_SERVICE_H_
